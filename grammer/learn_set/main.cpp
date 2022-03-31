#include <iostream>
#include <set>

using namespace std;

int main(){
    //定义
    set<int> t_set;
    t_set.clear();
    if(t_set.empty())
        cout<<"数值为空"<<endl;

    // 插入
    t_set.insert(1);
    t_set.insert(2);
    t_set.insert(3);
    t_set.insert(1);

    //earse
    t_set.erase(3);

    //size 
    cout<<"size = "<<t_set.size()<<endl;
    cout<<"max_size = "<<t_set.max_size()<<endl;


    //数据访问
    cout<<"size begin = "<<*t_set.begin()<<endl;
    cout<<"size end= "<<*t_set.end()<<endl;
    cout<<"count " <<t_set.count(1)<<endl;
    cout<<"3 count " <<t_set.count(3)<<endl;

    //遍历
    set<int> s;
    set<int>::iterator iter;
    for(int i = 0; i <= 5; ++i){
        s.insert(i);
    }

    for(iter = s.begin();iter!= s.end();++iter){
        cout<<"iter = "<<*iter<<" \n";
    }


    return 0;
}