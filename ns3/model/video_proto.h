#ifndef NS3_MPVIDEO_VIDEO_PROTO_H_
#define NS3_MPVIDEO_VIDEO_PROTO_H_
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <map>
#include <memory>
namespace zsy{
#define VIDEO_PACKET_SIZE 1000
enum Video_Proto_V1{
	v1_ping,
	v1_pong,
	v1_stream,
	v1_padding,
	v1_ack,
};
typedef struct{
	uint32_t fid;
	uint32_t timestamp;
	uint8_t index;
	uint8_t total;
	uint8_t ftype;
	uint16_t send_ts;
	uint16_t data_size;
	uint8_t		data[VIDEO_PACKET_SIZE];
}video_segment_t;
#define VIDEO_SEGMENT_HEADER_SIZE 15
class VideoPacketWrapper{
public:
	VideoPacketWrapper(uint32_t packet_id,uint32_t ref_time);
	~VideoPacketWrapper();
	void OnNewSegment(video_segment_t *seg,uint32_t now);
	bool IsStale(uint32_t now,uint32_t threshold);
	int16_t FType(){ return ftype_;};
	uint32_t get_packet_id(){ return packet_id_;}
	uint32_t get_payload_size() {return payload_size_;}
	uint32_t video_size(){
		return segment_->data_size;
	}
	int Serialize(uint8_t *buf,uint32_t len,uint32_t now);
	int64_t get_time_offset(){
		return send_ts_;
	}
	void set_enqueue_time(uint32_t now){
		enqueue_ts_=now;
	}
	uint32_t get_queue_delay(uint32_t now){
		return now-enqueue_ts_;
	}
	void set_pid(uint8_t pid){
		path_id_=pid;
	}
	uint8_t get_pid(){
		return path_id_;
	}
private:
	uint32_t packet_id_{0};
	uint32_t timestamp_{0};
	uint32_t generate_ts_{0};
	uint32_t ref_time_{0};
	int64_t send_ts_{-1};
	int16_t ftype_{-1};
	video_segment_t *segment_{NULL};
	uint32_t payload_size_{0};
	uint32_t enqueue_ts_{0};
	uint8_t path_id_{0};
};
class VideoPacketReceiveWrapper{
public:
	VideoPacketReceiveWrapper(uint32_t ts);
	~VideoPacketReceiveWrapper();
	void Deserialize(uint8_t*buf,uint32_t len);
	void set_first_ts(uint32_t first_ts){
		first_ts_=first_ts;
	}
	uint32_t get_owd();
    uint32_t get_sent_ts(){
    	return sent_ts_;
    }
    uint32_t get_frame_timestamp();
    int16_t get_index(){
    	int16_t index=-1;
    	if(segment_){
    		index=segment_->index;
    	}
    	return index;
    }
public:
	uint32_t received_ts_{0};
	uint32_t sent_ts_{0};
    uint32_t first_ts_{0};
	video_segment_t *segment_{NULL};
};
class VideoFrameBuffer{
public:
	VideoFrameBuffer(uint8_t ftype,uint32_t fid,uint8_t total);
	~VideoFrameBuffer(){};
	bool is_frame_completed(){
		return is_completed_;
	}
	bool is_expired(uint32_t now,uint32_t threshold);
	void OnNewPacket(std::shared_ptr<VideoPacketReceiveWrapper> packet,
			uint32_t ts);
	uint32_t get_frame_delay(){
		return max_recv_ts_-frame_timestamp_;
	}
	uint8_t Received(){
		return received_;
	}
	uint8_t Total(){
		return total_;
	}
private:
	uint8_t ftype_{255};
	uint32_t fid_{0};
	uint32_t frame_timestamp_{0};
	uint32_t max_recv_ts_{0};
	uint8_t received_{0};
	uint8_t total_{0};
	bool is_completed_{false};
	std::map<int16_t,std::shared_ptr<VideoPacketReceiveWrapper>> packets_;
};
}
#endif /* NS3_MPVIDEO_VIDEO_PROTO_H_ */
