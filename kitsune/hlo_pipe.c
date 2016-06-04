#include "hlo_pipe.h"
#include "task.h"
#include "uart_logger.h"


int hlo_stream_transfer_until(transfer_direction direction,
							hlo_stream_t * stream,
							uint8_t * buf,
							uint32_t buf_size,
							uint32_t transfer_delay,
							bool * flush ) {

	int ret, idx = 0;
	while(idx < buf_size){
		if(direction == INTO_STREAM){
			ret = hlo_stream_write(stream, buf+idx, buf_size - idx);
		}else{
			ret = hlo_stream_read(stream, buf+idx, buf_size - idx);
		}
		if( ret > 0 ) {
			idx += ret;
		}
		if(( flush && *flush )||(ret == HLO_STREAM_EOF)){
			DISP("  END %d %d \n\n", idx, ret);
			if( idx ){
				return idx;
			}else{
				return ret;
			}
		}else if(ret < 0){
			return ret;
		}else{
			if(idx == buf_size){
				return idx;
			}
			if(ret == 0){
				vTaskDelay(transfer_delay);
			}
		}
	}
	return HLO_STREAM_ERROR;
}

int hlo_stream_transfer_all(transfer_direction direction,
							hlo_stream_t * stream,
							uint8_t * buf,
							uint32_t buf_size,
							uint32_t transfer_delay){
	return hlo_stream_transfer_until( direction, stream, buf, buf_size, transfer_delay, NULL);
}
int hlo_stream_transfer_between_ext(
		hlo_stream_t * src,
		hlo_stream_t * dst,
		uint8_t * buf,
		uint32_t buf_size,
		uint32_t transfer_size,
		uint32_t transfer_delay){
	int transfer_count = 0;
	while( transfer_size > transfer_count ) {
		int ret = hlo_stream_transfer_all(FROM_STREAM, src, buf,buf_size, transfer_delay);
		if(ret < 0){
			return ret;
		}
		ret = hlo_stream_transfer_all(INTO_STREAM, dst, buf,ret,transfer_delay);
		if(ret < 0){
			return ret;
		}
		transfer_count += ret;
	}
	return transfer_count;
}
int hlo_stream_transfer_between(
		hlo_stream_t * src,
		hlo_stream_t * dst,
		uint8_t * buf,
		uint32_t buf_size,
		uint32_t transfer_delay){

	int ret = hlo_stream_transfer_all(FROM_STREAM, src, buf,buf_size, transfer_delay);
	if(ret < 0){
		return ret;
	}
	return hlo_stream_transfer_all(INTO_STREAM, dst, buf,ret,transfer_delay);

}

#define DBG_FRAMEPIPE(...)

#include "sl_sync_include_after_simplelink_header.h"
#include "crypto.h"
#include "hlo_proto_tools.h"
#include "streaming.pb.h"

int get_aes(uint8_t * dst);

void prep_for_pb(int type);

#define MAX_CHUNK_SIZE 512

static int id_counter = 0;


int frame_pipe_encode( pipe_ctx * pipe ) {
	uint8_t buf[MAX_CHUNK_SIZE+1];
	hlo_stream_t * obufstr = fifo_stream_open( MAX_CHUNK_SIZE+1 );

	uint8_t hmac[SHA1_SIZE] = {0};
	int ret;
	int transfer_delay = 100;
	Preamble preamble_data;
	hlo_stream_t * hmac_payload_str = NULL;
	size_t size;

	uint8_t key[AES_BLOCKSIZE];
	get_aes(key);

	if( pipe->hlo_pb_type == Preamble_pb_type_ACK ) {
		DBG_FRAMEPIPE(" enc ack\n");
		hlo_stream_end(pipe->source);
		ret = hlo_stream_transfer_between(pipe->source,pipe->sink,buf,sizeof(buf),4);
		goto enc_return;
	}

	//make the preamble
	preamble_data.type = pipe->hlo_pb_type;
	preamble_data.has_auth = true;
	preamble_data.auth = Preamble_auth_type_HMAC_SHA1;
	preamble_data.has_id = true;
	preamble_data.id = ++id_counter;
	//encode it
	ret = hlo_pb_encode(obufstr, Preamble_fields, &preamble_data);
	if( ret != 0 ) {
		LOGE(" enc pbh parse fail %d\n", ret);
		goto enc_return;
	}
	DBG_FRAMEPIPE(" enc start %d\n", pipe->flush);

	//read out the size of the pb payload from the source stream
	ret = hlo_stream_transfer_until(FROM_STREAM, pipe->source, (uint8_t*)&size,sizeof(size), transfer_delay, &pipe->flush);
	if(ret <= 0){
		goto enc_return;
	}
	//and write it out to the sink
	ret = hlo_stream_transfer_all(INTO_STREAM, obufstr, (uint8_t*)&size, sizeof(size),transfer_delay);
	if(ret <= 0){
		goto enc_return;
	}
	size = ntohl(size);
	DBG_FRAMEPIPE(" enc payload size %d\n", size);
	//now we can use the size to count down...

    //wrap the source in hmac stream
	hmac_payload_str = hlo_hmac_stream(obufstr, key, sizeof(key) );
	if(!hmac_payload_str){
		ret = HLO_STREAM_ERROR;
		goto enc_return;
	}
	//compute an hmac on each block until the end of the stream
	while( size != 0 ) {
		//full chunk unless we're on the last one
		size_t to_read = size > (MAX_CHUNK_SIZE-SHA1_SIZE) ? (MAX_CHUNK_SIZE-SHA1_SIZE) : size;

		DBG_FRAMEPIPE(" enc chunk size %d\n", to_read);

		ret = hlo_stream_transfer_between(pipe->source,hmac_payload_str,buf,to_read,4);
		if(ret <= 0){
			goto enc_return;
		}

		DBG_FRAMEPIPE(" enc hmac\n" );

		// grab the running hmac and drop it in the stream
		get_hmac( hmac, hmac_payload_str );
		ret = hlo_stream_transfer_until(INTO_STREAM, obufstr, hmac, sizeof(hmac), transfer_delay, &pipe->flush);
		if(ret <= 0){
			goto enc_return;
		}

		DBG_FRAMEPIPE(" enc drop\n" );

		// drop the coalescing stream
		hlo_stream_end(obufstr);
		ret = hlo_stream_transfer_between(obufstr,pipe->sink,buf,sizeof(buf),4);
		if(ret <= 0){
			goto enc_return;
		}
		//prevent underflow
		if( size < to_read ) {
			LOGE("!enc underflow %d %d\n", size, to_read);
			ret = HLO_STREAM_ERROR;
			goto enc_return;
		}
		size -= to_read;
	}

	enc_return:
	hlo_stream_close(obufstr);
	if( hmac_payload_str ) {
		hlo_stream_close(hmac_payload_str);
	}
	DBG_FRAMEPIPE( " enc returning %d\n", ret );
	return ret;
}
int frame_pipe_decode( pipe_ctx * pipe ) {
	uint8_t buf[512];
	uint8_t hmac[2][SHA1_SIZE] = {0};
	int ret;
	size_t size;
	int transfer_delay = 100;
	Preamble preamble_data;
	hlo_stream_t * hmac_payload_str = NULL;
	uint8_t key[AES_BLOCKSIZE];
	get_aes(key);

	dec_again:
	//read out the pb preamble
	ret = hlo_pb_decode( pipe->source, Preamble_fields, &preamble_data );
	if( ret != 0 ) {
		LOGE("    dec pbh parse fail %d\n", ret);
		goto dec_return;
	}
	DBG_FRAMEPIPE("    dec type  %d\n", preamble_data.type );

	//ack has no body or hmac
	if( preamble_data.type == Preamble_pb_type_ACK ) {
		DBG_FRAMEPIPE("\t  rx ack %d\n", preamble_data.id );
		goto dec_again;
	}
	//notify the decoder what kind of pb is coming
	prep_for_pb(preamble_data.type);

	//read out the pb payload size
	ret = hlo_stream_transfer_all( FROM_STREAM, pipe->source, (uint8_t*)&size,sizeof(size), transfer_delay );
	if(ret <= 0){
		goto dec_return;
	}
	DBG_FRAMEPIPE("    dec payload size %d\n", ntohl(size) );
	//write it for the pb parser
	ret = hlo_stream_transfer_all( INTO_STREAM, pipe->sink, (uint8_t*)&size,sizeof(size),transfer_delay);
	size = ntohl(size);
	if(ret <= 0){
		goto dec_return;
	}
	DBG_FRAMEPIPE("    dec hmac begin\n" );

	//wrap the source stream in an hmac stream and read the payload
	hmac_payload_str = hlo_hmac_stream(pipe->source, key, sizeof(key) );
	if(!hmac_payload_str){
		ret = HLO_STREAM_ERROR;
		goto dec_return;
	}

	//compute an hmac on each block until the end of the stream
	while( size != 0 ) {
		//full chunk unless we're on the last one
		size_t to_read = size > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : size;
		DBG_FRAMEPIPE("    dec reading %d\n", to_read );

		ret = hlo_stream_transfer_all( FROM_STREAM, hmac_payload_str, buf,to_read, transfer_delay );
		if(ret <= 0){
			goto dec_return;
		}

		//read in and verify the HMAC
		ret = hlo_stream_transfer_all( FROM_STREAM, pipe->source, hmac[1],sizeof(hmac[1]), transfer_delay );
		if(ret <= 0){
			goto dec_return;
		}
		DBG_FRAMEPIPE("    dec cmpt hmac\n"  );

		get_hmac( hmac[0], hmac_payload_str );
		if( memcmp(hmac[0], hmac[1], sizeof(hmac[1]))) {
			LOGE("    dec hmac mismatch\n");
			ret = HLO_STREAM_ERROR;
			goto dec_return;
		}
		if(ret <= 0){
			goto dec_return;
		}

		DBG_FRAMEPIPE("    dec parsing\n"  );
		//dump the pb payload out to the parser
		ret = hlo_stream_transfer_all(INTO_STREAM, pipe->sink, buf,to_read,transfer_delay);
		if(ret <= 0){
			goto dec_return;
		}
		//prevent underflow
		if( size < ret ) {
			LOGE("!dec underflow %d %d\n", size, ret);
			ret = HLO_STREAM_ERROR;
			goto dec_return;
		}
		size -= ret;
	}
	{
		hlo_stream_t * obufstr = fifo_stream_open( 64 );
		DBG_FRAMEPIPE("\t  ack %d sending\n", preamble_data.id);

		preamble_data.type = Preamble_pb_type_ACK;
		ret = hlo_pb_encode(obufstr, Preamble_fields, &preamble_data);
		if( ret != 0 ) {
			hlo_stream_close(obufstr);
			LOGE("\t  ack %d encode fail\n", preamble_data.id);
			ret = HLO_STREAM_ERROR;
			goto dec_return;
		}
		hlo_stream_end( obufstr );
		ret = hlo_stream_transfer_between(obufstr,pipe->source,buf,sizeof(buf),4);
		hlo_stream_close(obufstr);
		if( ret <= 0 ) {
			LOGE("\t  ack %d send fail\n", preamble_data.id);
			ret = HLO_STREAM_ERROR;
			goto dec_return;
		}
		DBG_FRAMEPIPE("\t  ack %d sent\n", preamble_data.id);
	}

	dec_return:
	if( hmac_payload_str ) {
		hlo_stream_close(hmac_payload_str);
	}
	DBG_FRAMEPIPE("    dec returning %d\n", ret );
	return ret;
 }
void thread_frame_pipe_encode(void* ctx) {
	pipe_ctx * pctx = (pipe_ctx*)ctx;

	frame_pipe_encode( pctx );
	xSemaphoreGive(pctx->join_sem);
	vTaskDelete(NULL);
}
void thread_frame_pipe_decode(void* ctx) {
	pipe_ctx * pctx = (pipe_ctx*)ctx;

	frame_pipe_decode( pctx );
	hlo_stream_end(pctx->sink);
	xSemaphoreGive(pctx->join_sem);
	vTaskDelete(NULL);
}
