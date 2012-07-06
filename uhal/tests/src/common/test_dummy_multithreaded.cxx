#include "uhal/uhal.hpp"
#include "uhal/tests/tools.hpp"

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 

#include <iostream>
#include <cstdlib>
#include <typeinfo>

using namespace uhal;

#define N_10_THREADS     10
#define N_1k_ITERATIONS  1000
#define N_1MB_SIZE       1024*1024/4
#define TIMEOUT_S        50  

void job(const std::string& connection, const std::string& id) {
  ConnectionManager manager(connection);

  HwInterface hw=manager.getDevice(id);
  
  hw.ping();

  uint32_t x = static_cast<uint32_t>(rand());

  hw.getNode("REG").write(x);
  ValWord< uint32_t > reg = hw.getNode("REG").read();
 
  CACTUS_CHECK(!reg.valid());
  CACTUS_TEST_THROW(reg.value(),uhal::exception);

  std::vector<uint32_t> xx;
  for(size_t i=0; i!= N; ++i)
    xx.push_back(static_cast<uint32_t>(rand()));
  
  hw.getNode("MEM").writeBlock(xx);
  ValVector< uint32_t > mem = hw.getNode("MEM").readBlock(N);

  CACTUS_CHECK(!mem.valid());
  CACTUS_CHECK(mem.size() == N);
  CACTUS_TEST_THROW(mem.at(0),uhal::exception);
    
  CACTUS_TEST(hw.dispatch());
  
  CACTUS_CHECK(mem.valid());
  CACTUS_CHECK(mem.value() == x);

  bool correct_block_write_read = true;
  ValVector< uint32_t >::const_iterator i=mem.begin();
  std::vector< uint32_t >::const_iterator j=xx.begin();
  for(ValVector< uint32_t >::const_iterator i(mem.begin());i!=mem.end();++i , ++j) {
    correct_block_write_read = correct_block_write_read && (*i == *j);
  }
  
  CACTUS_CHECK(correct_block_write_read);
}

void launch_threads(size_t n_threads,connection_file,device_id) {
  std::vector<boost::thread *> jobs;

  for(size_t i=0; i!=n_threads; ++i) {
    jobs.push_back(new boost::thread(job,connection_file,device_id));
  }
  
  boost::posix_time::time_duration timeout = boost::posix_time::seconds(TIMEOUT_S);
  for(size_t i=0; i!=n_threads; ++i) {
    CHECK(jobs[i]->timed_join(timeout));
    delete jobs;
  }
}

int main ( int argc,char* argv[] )
{
  std::map<std::string,std::string> params = tests::default_arg_parsing(argc,argv);
  
  std::string connection_file = params["connection_file"];
  std::string device_id = params["device_id"];
  std::cout << "STARTING TEST " << argv[0] << " (connection_file='" << connection_file<<"', device_id='" << device_id << "')..." << std::endl;
  
  CACTUS_TEST(launch_threads(N_10_THREADS,connection_file,device_id));
  
  return 0;
}