#include "t_generator.h"
namespace ns3{
TGenerator::TGenerator(uint32_t start_bps,uint32_t fs){
	min_bitrate_=start_bps;
	rate_=min_bitrate_;
	fs_=fs;
	delta_=1000/fs_;
}
void TGenerator::RegisterTarget(DataTarget *t){
	t_=t;
}
void TGenerator::ChangeRate(uint32_t bitrate){
	if(bitrate<min_bitrate_){
		rate_=min_bitrate_;
	}
	else{
		rate_=bitrate;
	}
	if(!m_rate_cb_.IsNull()){
		m_rate_cb_(bitrate);
	}
}
void TGenerator::SendFrame(){
	if(!running_){
		return;
	}
	uint32_t now=Simulator::Now().GetMilliSeconds();
	uint32_t elapse=0;
	if(last_send_ts_==0){
		elapse=delta_;
	}else{
		elapse=now-last_send_ts_;
	}
	uint32_t len=0;
	len=elapse*rate_/(1000*8);
	if(t_){
		uint8_t *buf=new uint8_t[len];
		t_->OnFrame(buf,len);
		delete [] buf;
	}
	last_send_ts_=now;
}
void TGenerator::Generate()
{
	if(m_timer.IsExpired())
	{
		SendFrame();
		Time next=MilliSeconds(delta_);
		m_timer=Simulator::Schedule(next,&TGenerator::Generate,this);
	}
}
void TGenerator::Start(){
	if(!running_){
		m_timer=Simulator::ScheduleNow(&TGenerator::Generate,this);
	}
	running_=true;
}
void TGenerator::Stop(){
	running_=false;
	m_timer.Cancel();
}
}




