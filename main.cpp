/* 
 * File:   main.cpp
 * Author: allen
 *
 * Created on 2013年10月4日, 下午3:32
 */

#include <cstdlib>
#include<cmath>
#include<iostream>
#include <boost/thread/thread.hpp>
#include"com/BioCom.h"
#include"unistd.h"
using namespace std;
using namespace biocom;
void sendTest(){
    cerr<<"send"<<endl;
    int i = 100;
    while(--i){
    CM.setJointsCmd(RSHANK, 20.0f*sin(i*2*M_PI/100.0f));
    CM.printJointsCmd();
    CM.sendCom();
  CM.printJointsCmd();
    sleep(1);
    }
}

void recieveTest(){
    cerr<<"receive"<<endl;
    CM.recieveCom();
}

void printInfo(){
    cerr<<"print Info"<<endl;
    int i =100;
     while(--i){
    CM.printJointsInfo();
    sleep(1);
     }
}
/*
 * 
 */
int main(int argc, char** argv) {
cout<<"Start run threads"<<endl;

	boost::thread_group ctrThrdGroup;
        ctrThrdGroup.create_thread(&sendTest);
       ctrThrdGroup.create_thread(&recieveTest);
       ctrThrdGroup.create_thread(&printInfo);
        ctrThrdGroup.join_all();
        
   
    return 0;
}

