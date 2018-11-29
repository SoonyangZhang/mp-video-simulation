#include "fakevideogenerator.h"
#include "rtc_base/timeutils.h"
#include <string>
#include "ns3/log.h"
#include "ns3/simulator.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE ("FakeVideoGenerator");
FakeVideoGenerator::FakeVideoGenerator(uint32_t minbitrate,uint32_t fs)
:min_bitrate_(minbitrate)
,fs_(fs)
,rate_(min_bitrate_)
,first_ts_(0)
,last_send_ts_(0)
,delta_(1)
,frame_id_(0)
,running_(false)
,sender_(NULL){
	delta_=1000/fs_;
	ratecontrol_.RegisterRateChangeCallback(this);
    double fps=fs_;
    auto innerCodec = new syncodecs::StatisticsCodec{fps};
    //auto codec = new syncodecs::ShapedPacketizer{innerCodec, 1000};
	m_codec.reset(innerCodec);
    double codec_rate=min_bitrate_;
	m_codec->setTargetRate (codec_rate);
}
FakeVideoGenerator::~FakeVideoGenerator(){
    if(schedule_){
        delete schedule_;
    }
}
void FakeVideoGenerator::ChangeRate(uint32_t bitrate){
	if(bitrate<min_bitrate_){
		rate_=min_bitrate_;
	}
	else{
		rate_=bitrate;
	}
	uint32_t now=Simulator::Now().GetMilliSeconds();
	if(first_ts_==0){
		first_ts_=now;
	}
	if(!m_rate_cb_.IsNull()){
		m_rate_cb_(bitrate);
	}
    double codec_rate=rate_;
	m_codec->setTargetRate (codec_rate);
	//uint32_t elapse=now-first_ts_;
	//NS_LOG_INFO("t "<<elapse<<" r "<<rate_);
}
void FakeVideoGenerator::ConfigureSchedule(std::string type){
    if(type==std::string("scale")){
        schedule_=new ScaleSchedule(CostType::c_intant);
    }else if(type==std::string("wrr")){
        schedule_=new WrrSchedule(CostType::c_intant);
    }else if(type==std::string("edcld8")){
        schedule_=new EDCLDSchedule(0.8);
    }else if(type==std::string("edcld")){
        schedule_=new EDCLDSchedule(0.2);
    }else if(type==std::string("bc")){
         schedule_=new BalanceCostSchedule(CostType::c_intant);
    }else if(type==std::string("sfl")){
         schedule_=new SFLSchedule();
    }else if(type==std::string("water")){
        schedule_=new WaterFillingSchedule();
    }else if(type==std::string("minq")){
         schedule_=new MinQueueSchedule(CostType::c_intant);
    }
}
void FakeVideoGenerator::RegisterSender(SenderInterface *s){
	sender_=s;
	sender_->RegisterPacketSource(this);
	sender_->SetRateControl(&ratecontrol_);
    NS_ASSERT(schedule_!=NULL);
	sender_->SetSchedule(schedule_);
}
void FakeVideoGenerator::SendFrame(uint32_t frame_len){
	if(!running_){
		return;
	}
	uint32_t f_type=0;
	uint32_t len=0;
	if(frame_id_==0){
		f_type=1;
	}
	frame_id_++;
	len=frame_len;
    uint32_t packet_size=len/1000;
    //printf("packet_size %d\t%d\n",frame_id_-1,packet_size);
	if(sender_){
		uint8_t *buf=new uint8_t[len];
		sender_->SendVideo(0,f_type,buf,len);
		delete [] buf;
	}
}
void FakeVideoGenerator::Generate()
{
	if(m_timer.IsExpired())
	{
	    syncodecs::Codec& codec = *m_codec;
        double codec_rate=rate_;
	    codec.setTargetRate (codec_rate);
	    ++codec;
	    const auto bytesToSend = codec->first.size ();
        NS_LOG_INFO("rate "<<std::to_string(codec.getTargetRate())<<" "<<std::to_string(bytesToSend));
		SendFrame(bytesToSend);
		auto secsToNextFrame = codec->second;
		Time Next{Seconds(secsToNextFrame)};
		m_timer=Simulator::Schedule(Next,&FakeVideoGenerator::Generate,this);
	}
}
void FakeVideoGenerator::Start(){
	if(!running_){
		m_timer=Simulator::ScheduleNow(&FakeVideoGenerator::Generate,this);
	}
	running_=true;
}
void FakeVideoGenerator::Stop(){
	running_=false;
	m_timer.Cancel();
}
}




