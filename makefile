mk_cxx=g++

target_home=bin
target=MyFavoriteServer

src_path=src
kafka_library_path=include/Kafka
code_library_path=include/Code_Library/platform_cross
wfshared_path=include/WFShared
zjsshared_path=include/ZJSShared
cppjson_path=include/json
tinyxml_path=include/tinyxml
include_path=-I. \
	-I$(src_path) \
	-I$(kafka_library_path)\
	-I$(code_library_path)\
	-I$(wfshared_path)\
	-I$(zjsshared_path)\
	-I$(tinyxml_path)\
            
src=\
	$(wildcard $(src_path)/*.cpp) \
	$(wildcard $(src_path)/*.c)\
	$(wildcard $(tinyxml_path)/*.cpp)\
	$(wildcard $(cppjson_path)/*.cpp)

cflags= -w -Wall -g $(include_path) 
headers=$(wildcard $(src_path)/*.h) 
objs=$(patsubst %.c,%.o,$(patsubst %.cpp, %.o,$(patsubst %.cc,%.o,$(src))))

static_lib= ./lib/liburlfea.a ./lib/librdkafka++.a ./lib/librdkafka.a
dynamic_lib= -L.  -pthread  -lz -lrt  -lz  -lpthread -luuid 

$(target):$(objs) $(headers) $(src)
	echo $(objs) $(headers) $(src)
	$(mk_cxx) -o $(target) $(objs) $(static_lib) $(dynamic_lib)
	rm $(objs)
	-mv $(target) $(target_home)
%.o:%.c $(headers)
	$(mk_cxx) $(cflags) -c $< -o $@ 
%.o:%.cpp $(headers)
	$(mk_cxx) $(cflags) -c $< -o $@ 
%.o:%.cc $(headers)
	$(mk_cxx) $(cflags) -c $< -o $@ 
	
clean:
	-rm $(target_home)/$(target)
