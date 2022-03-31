#include <iostream>
#include <map>

using namespace std;

int main(){
    //定义
    map<int,int> t_map;

    // 插入
    t_map.insert(pair<int,int>(1,-1));
    t_map.insert(pair<int,int>(2,-2));
    t_map.insert(pair<int,int>(3,-3));

    //earse
    t_map.erase(3);

    // //size 
    // cout<<"size = "<<t_set.size()<<endl;
    // cout<<"max_size = "<<t_set.max_size()<<endl;


    // //数据访问
    // cout<<"size begin = "<<*t_set.begin()<<endl;
    // cout<<"size end= "<<*t_set.end()<<endl;
    // cout<<"count " <<t_set.count(1)<<endl;
    // cout<<"3 count " <<t_set.count(3)<<endl;

    //遍历
    map<int,int>::iterator iter;

    for(iter = t_map.begin();iter!= t_map.end();++iter){
        cout<<"key = "<<iter->first<<" value="<<iter->second<<endl;
    }


    return 0;
}