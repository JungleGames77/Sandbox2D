#include "SaveSystem.hpp"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <limits>
#include <system_error>

namespace sandbox {
namespace {
namespace fs=std::filesystem;
constexpr std::uint32_t VERSION=1;
constexpr char MAGIC[8]={'S','B','2','D','S','A','V','E'};
fs::path saveFolder(){
#ifdef _WIN32
    if(const char* appdata=std::getenv("APPDATA")) return fs::path(appdata)/"Sandbox2D"/"worlds";
#endif
    return fs::current_path()/"saves";
}
fs::path savePath(const std::string& id){return saveFolder()/(id+".sbw");}
bool validId(const std::string& id){
    return !id.empty() && id.size()<80 && std::all_of(id.begin(),id.end(),[](unsigned char c){return (c>='0'&&c<='9')||(c>='a'&&c<='z')||c=='_';});
}
template<class T> bool put(std::ostream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));return bool(out);}
template<class T> bool take(std::istream& in,T& value){in.read(reinterpret_cast<char*>(&value),sizeof(value));return bool(in);}
bool putStack(std::ostream& out,const ItemStack& stack){
    return put(out,static_cast<std::uint8_t>(stack.block))&&put(out,static_cast<std::int32_t>(stack.count));
}
bool takeStack(std::istream& in,ItemStack& stack){
    std::uint8_t b;std::int32_t n;
    if(!take(in,b)||!take(in,n)||b>StoneAxe||n<0||n>MAX_STACK||((n==0)!=(b==Air))||(n>maxStack(static_cast<Block>(b)))) return false;
    stack={static_cast<Block>(b),n};return true;
}
bool readName(std::istream& in,std::string& name){
    std::uint32_t n=0;
    if(!take(in,n)||n==0||n>160) return false;
    name.resize(n);in.read(name.data(),n);return bool(in);
}
}
std::string createWorldId(){
    auto tick=std::chrono::system_clock::now().time_since_epoch().count();
    for(int n=0;n<10000;++n){auto id="world_"+std::to_string(tick)+"_"+std::to_string(n);if(!fs::exists(savePath(id)))return id;}
    return {};
}
bool saveWorld(const std::string& id,const std::string& name,const WorldSave& s){
    if(!validId(id)||name.empty()||name.size()>160||s.world.size()!=WORLD_W*WORLD_H||s.selectedSlot<0||s.selectedSlot>=HOTBAR_SIZE||!std::isfinite(s.cameraZoom))return false;
    std::error_code ec;fs::create_directories(saveFolder(),ec);if(ec)return false;
    const auto path=savePath(id), temp=fs::path(path.string()+".tmp");
    {
        std::ofstream out(temp,std::ios::binary|std::ios::trunc);if(!out)return false;
        out.write(MAGIC,8);
        if(!put(out,VERSION)||!put(out,static_cast<std::uint32_t>(name.size())))return false;
        out.write(name.data(),static_cast<std::streamsize>(name.size()));
        if(!put(out,static_cast<std::uint32_t>(WORLD_W))||!put(out,static_cast<std::uint32_t>(WORLD_H)))return false;
        for(Block b:s.world)if(!put(out,static_cast<std::uint8_t>(b)))return false;
        for(float f:{s.player.position.x,s.player.position.y,s.player.velocity.x,s.player.velocity.y})if(!put(out,f))return false;
        if(!put(out,static_cast<std::uint8_t>(s.player.grounded?1:0)))return false;
        for(const auto& stack:s.inventory)if(!putStack(out,stack))return false;
        for(const auto& stack:s.crafting)if(!putStack(out,stack))return false;
        if(!putStack(out,s.cursor)||!put(out,static_cast<std::int32_t>(s.selectedSlot))||!put(out,s.cameraZoom))return false;
        out.flush();if(!out)return false;
    }

    const auto backup=fs::path(path.string()+".bak");
    if(fs::exists(path)){
        fs::remove(backup,ec);ec.clear();fs::rename(path,backup,ec);if(ec)return false;
    }
    ec.clear();fs::rename(temp,path,ec);
    if(ec){std::error_code ignored;if(fs::exists(backup))fs::rename(backup,path,ignored);return false;}
    fs::remove(backup,ec);return true;
}
bool loadWorld(const std::string& id,std::string& name,WorldSave& s){
    if(!validId(id))return false;
    std::ifstream in(savePath(id),std::ios::binary);if(!in)return false;
    char magic[8];in.read(magic,8);if(!in||std::memcmp(magic,MAGIC,8)!=0)return false;
    std::uint32_t version=0,w=0,h=0;
    std::string newName;WorldSave next;
    if(!take(in,version)||version!=VERSION||!readName(in,newName)||!take(in,w)||!take(in,h)||w!=WORLD_W||h!=WORLD_H)return false;
    next.world.resize(WORLD_W*WORLD_H);
    for(auto& b:next.world){std::uint8_t n;if(!take(in,n)||n>StoneAxe)return false;b=static_cast<Block>(n);}
    for(float* f:{&next.player.position.x,&next.player.position.y,&next.player.velocity.x,&next.player.velocity.y})if(!take(in,*f)||!std::isfinite(*f))return false;
    std::uint8_t grounded=0;if(!take(in,grounded)||grounded>1)return false;next.player.grounded=grounded!=0;
    for(auto& stack:next.inventory)if(!takeStack(in,stack))return false;
    for(auto& stack:next.crafting)if(!takeStack(in,stack))return false;
    std::int32_t selected=0;
    if(!takeStack(in,next.cursor)||!take(in,selected)||selected<0||selected>=HOTBAR_SIZE||!take(in,next.cameraZoom)||!std::isfinite(next.cameraZoom)||next.cameraZoom<0.5f||next.cameraZoom>2.5f)return false;
    next.selectedSlot=selected;
    name=std::move(newName);s=std::move(next);return true;
}
std::vector<SavedWorldInfo> listWorlds(){
    std::vector<SavedWorldInfo> result;std::error_code ec;
    if(!fs::exists(saveFolder(),ec))return result;
    for(const auto& entry:fs::directory_iterator(saveFolder(),fs::directory_options::skip_permission_denied,ec)){
        if(ec)break;
        if(!entry.is_regular_file()||entry.path().extension()!=".sbw")continue;
        auto id=entry.path().stem().string();
        if(!validId(id))continue;
        std::ifstream in(entry.path(),std::ios::binary);char magic[8];std::uint32_t version=0;std::string name;
        in.read(magic,8);
        if(in&&std::memcmp(magic,MAGIC,8)==0&&take(in,version)&&version==VERSION&&readName(in,name))result.push_back({id,name});
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.id>b.id;});
    return result;
}
}
