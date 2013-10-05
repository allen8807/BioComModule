/* 
 * File:   BioCom.h
 * Author: allen
 *
 * Created on 2013年10月4日, 下午3:34
 */

#ifndef BIOCOM_H
#define	BIOCOM_H
#include"Singleton.hpp"
#include<map>
#define MAX_BUFFER_SIZE (0xff)
namespace biocom {

    enum IDEV_ID {
        HEAD = 18,
        NECK = 19,
        RSHOULDER = 12,
        RUPPERARM = 13,
        RLOWERARM = 14,
        LSHOULDER = 15,
        LUPPERARM = 16,
        LLOWERARM = 17,
        RHIP1 = 0,
        RHIP2 = 1,
        RTHIGH = 2,
        RSHANK = 3,
        RANKLE = 4,
        RFOOT = 5,
        LHIP1 = 6,
        LHIP2 = 7,
        LTHIGH = 8,
        LSHANK = 9,
        LANKLE = 10,
        LFOOT = 11,
        DOF = 20,
        RFRS = 30,
        LFRS = 31,
        GRY = 40,
        ALL = 23
    };

    struct GRYData {
        float x;
        float y;
        float z;
        bool flag;
    };

    struct FRSData {
        float lu;
        float ru;
        float rd;
        float ld;
        bool flag;
    };

    class BioCom :
    public Singleton<BioCom> {
        typedef std::map<IDEV_ID, float> BCJoints;
    public:
        BioCom();
        virtual ~BioCom();

        void setJointsCmd(IDEV_ID p_id, float p_value) {
            mJointsCmd[p_id] = p_value;
        }
        int sendCom();
        int recieveCom();

        void printJointsCmd();
        void printJointsInfo();
    private:
        float calcValue(unsigned char p_high, unsigned char p_low);
        float calcFRSValue(unsigned char p_high, unsigned char p_low);
        bool initCom();
        void initData();



        void parserBuff(unsigned char p_buffLen);
        unsigned char makeBuff();
    private:

        BCJoints mJointsCmd;

        BCJoints mJointsInfo;
        GRYData mGRYInfo;
        FRSData mRFRSInfo;
        FRSData mLFRSInfo;


        unsigned char mSBuff[MAX_BUFFER_SIZE];
        unsigned char mRBuff[MAX_BUFFER_SIZE];

        int mComFd; //device file description

    };
#define CM biocom::BioCom::GetSingleton()
}
#endif	/* BIOCOM_H */

