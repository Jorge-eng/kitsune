#include "gtest/gtest.h"
#include "../kitsune/tensor/net_stats.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "../kitsune/protobuf/keyword_stats.pb.h"
#include "../kitsune/nanopb/pb_encode.h"

extern "C" {
    void set_encoders_with_data(KeywordStats * keyword_stats_item, NetStats_t * stats);
}

class TestNetStats : public ::testing::Test {
protected:
    virtual void SetUp() {
        
    }
    
    virtual void TearDown() {
        
    }
    
    
};


TEST_F(TestNetStats,TestAccumulationOfStats) {
 
    NetStats_t stats;
    
    net_stats_init(&stats, 3, "foobars");
    Weight_t output [3] = {0,0,1<<QFIXEDPOINT};
    
    net_stats_update_counts(&stats, output);
    
    ASSERT_TRUE(stats.counts[0][0] == 1);
    ASSERT_TRUE(stats.counts[0][1] == 0);
    ASSERT_TRUE(stats.counts[1][0] == 1);
    ASSERT_TRUE(stats.counts[2][0] == 0);
    ASSERT_TRUE(stats.counts[2][NET_STATS_HISTOGRAM_BINS - 1] == 1);

    
}

typedef struct {
    char buf[1024];
} Buf_t;

bool callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    Buf_t * item = (Buf_t*) stream->state;
    memcpy(&item->buf[stream->bytes_written],buf,count);
    return true;
}

bool callback_file_write(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    FILE *file = (FILE*) stream->state;
    return fwrite(buf, 1, count, file) == count;
}

TEST_F(TestNetStats,TestProtobuf) {

    NetStats_t stats;
    
    net_stats_init(&stats, 3, "foobars");
    Weight_t output [3] = {0,0,1<<QFIXEDPOINT};
    
    net_stats_update_counts(&stats, output);
    
    net_stats_record_activation(&stats, 1, 42);
    net_stats_record_activation(&stats, 2, 85);
    net_stats_record_activation(&stats, 3, 93);

    KeywordStats pb_kwstats;
    memset(&pb_kwstats,0,sizeof(KeywordStats));
 
 
    set_encoders_with_data(&pb_kwstats,&stats);
    
    /*
    Buf_t state;
    memset(&state,0,sizeof(state));
    
    pb_ostream_t bufstream = {&callback, &state, 1024, 0};
    pb_encode(&bufstream, KeywordStats_fields, &pb_kwstats);
    */
    
    FILE * f = fopen("test001.dat", "wb");
    pb_ostream_t fileoutstream = {&callback_file_write, f, SIZE_MAX, 0};
    pb_encode(&fileoutstream, KeywordStats_fields, &pb_kwstats);

    fclose(f);
    
}

