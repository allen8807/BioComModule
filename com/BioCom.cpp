/* 
 * File:   BioCom.cpp
 * Author: allen
 * 
 * Created on 2013年10月4日, 下午3:34
 */

#include "BioCom.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
namespace biocom {

    BioCom::BioCom() {
        mJointsInfo.clear();
        mJointsCmd.clear();
        memset(mSBuff, 0, sizeof (unsigned char) *MAX_BUFFER_SIZE);
        memset(mRBuff, 0, sizeof (unsigned char) *MAX_BUFFER_SIZE);
        initData();

        initCom();
    }

    BioCom::~BioCom() {

        if (mComFd != -1) {
            if (-1 == close(mComFd)) {
                perror("Close the Device failure!\n");
            }

        }
    }

    void BioCom::initData() {
        memset(&mGRYInfo, 0, sizeof (GRYData));
        memset(&mRFRSInfo, 0, sizeof (FRSData));
        memset(&mLFRSInfo, 0, sizeof (FRSData));
    }

    bool BioCom::initCom() {
        struct termios option;

        mComFd = open("/dev/ttyS0", O_RDWR | O_NOCTTY); /*读写方式打开串口*/
        perror("open /dev/ttyS0");
        if (mComFd == -1) /*打开失败*/ {
            perror("open /dev/ttyS0 failed!");
            return false;
        }
        //printf("ready for sending data...\n"); /*准备开始发送数据*/

        tcgetattr(mComFd, &option);
        cfmakeraw(&option);
        cfsetispeed(&option, B115200); /*波特率设置为 115200bps*/
        cfsetospeed(&option, B115200);
        /*******************************************************************/
        tcsetattr(mComFd, TCSANOW, &option);
    }

    /**
     * protocol
     * 0xff 0xff (size of data) (data)(sumcheck)
     * data
     * (id) (high value) (low value)
     * date example
     * 0x01 0x80 0x80
     * id is 1, pos is -0.64
     *@return number of send bytes
     */
    int BioCom::sendCom() {
        if (mJointsCmd.empty()) {
            return -1;
        }
        memset(mSBuff, 0, sizeof (unsigned char) *MAX_BUFFER_SIZE);
        unsigned char bufSize = makeBuff();
        int retv = 0;
        retv = write(mComFd, mSBuff, bufSize);
        mJointsCmd.clear();
        if (-1 == retv) {
            perror("write error");
            return -1;
        }

        return retv;
    }

    /**
     * protocol
     * 0xff 0xff (size of data) (data) (sumcheck)
     * data
     * DuoJi
     * (id)(state)(high value) (low value)
     * GRY
     *
     * @return
     */
    int BioCom::recieveCom() {
        int retv = 0;
        unsigned char len = 0;
        while (true) {
            memset(mRBuff, 0, sizeof (unsigned char) *MAX_BUFFER_SIZE);
            retv = read(mComFd, mRBuff, 1);
            if (retv == -1) {
                perror("read error"); /*读状态标志判断*/
            }
            if (0xff == mRBuff[0]) {
                retv = read(mComFd, mRBuff, 1);
                //printf("sec %x\t",mRBuff[0]);
                if (retv == -1) {
                    perror("read error"); /*读状态标志判断*/
                }
                if (0xff == mRBuff[0]) {
                    retv = read(mComFd, mRBuff, 1);
                    if (retv == -1) {
                        perror("read error"); /*读状态标志判断*/
                    }
                    len = mRBuff[0];
                } else {
                    continue;
                }
            } else {
                continue;
            }

            retv = read(mComFd, mRBuff, len);
            if (retv == -1) {
                perror("read error"); /*读状态标志判断*/
            }
            //和校验
            //printf("len: %x\n",len);
            unsigned char sumcheck = len;
            int i = 0;
            for (i = 0; i < len - 1; ++i) {
                sumcheck += mRBuff[i];
                //printf("%x\t",mRBuff[i]);
            }
            //printf("%x check%x\n",mRBuff[len - 1],sumcheck);
            if (sumcheck != mRBuff[len - 1]) {
                perror("recieve sumcheck error");
                continue;
            }

            parserBuff(len);
        }

    }

    /**
     *
     * @param p_buffLen the size of the buff
     */
    void BioCom::parserBuff(unsigned char p_buffLen) {
        int i = 0;
        mJointsInfo.clear();
        initData();
        for (i = 0; i < p_buffLen - 1; ++i) {
            if (mRBuff[i] < DOF && mRBuff[i] > 0) {
                if (mRBuff[i + 1] == 0x00) {
                    this->mJointsInfo[(IDEV_ID) (mRBuff[i])] = calcValue(mRBuff[i + 2], mRBuff[i + 3]);
                    i += 3;
                } else {
                    ++i;
                }
            } else if (RFRS == mRBuff[i]) {
                mRFRSInfo.lu = calcFRSValue(mRBuff[i + 1], mRBuff[i + 2]);
                mRFRSInfo.ru = calcFRSValue(mRBuff[i + 3], mRBuff[i + 4]);
                mRFRSInfo.rd = calcFRSValue(mRBuff[i + 5], mRBuff[i + 6]);
                mRFRSInfo.ld = calcFRSValue(mRBuff[i + 7], mRBuff[i + 8]);
                mRFRSInfo.flag = true;
            } else if (LFRS == mRBuff[i]) {
                mLFRSInfo.lu = calcFRSValue(mRBuff[i + 1], mRBuff[i + 2]);
                mLFRSInfo.ru = calcFRSValue(mRBuff[i + 3], mRBuff[i + 4]);
                mLFRSInfo.rd = calcFRSValue(mRBuff[i + 5], mRBuff[i + 6]);
                mLFRSInfo.ld = calcFRSValue(mRBuff[i + 7], mRBuff[i + 8]);
                mLFRSInfo.flag = true;
            } else if (GRY == mRBuff[i]) {
                mGRYInfo.x = calcValue(mRBuff[i + 1], mRBuff[i + 2]);
                mGRYInfo.y = calcValue(mRBuff[i + 3], mRBuff[i + 4]);
                mGRYInfo.z = calcValue(mRBuff[i + 5], mRBuff[i + 6]);
                mGRYInfo.flag = true;
            }
        }
    }

    /**
     * 
     * @return the size of the buff which was made 
     */
    unsigned char BioCom::makeBuff() {
        int len = mJointsCmd.size();
        unsigned char sumCheck = 0;
        unsigned char bufSize = 3 * len + 4;

        mSBuff[0] = 0xff;
        mSBuff[1] = 0xff;
        mSBuff[2] = bufSize - 3;
        sumCheck += mSBuff[2];
        int i = 3;
        int temp = 0;
        for (BCJoints::const_iterator iter = mJointsCmd.begin();
                iter != mJointsCmd.end();
                ++iter) {
            mSBuff[i] = (unsigned char) iter->first;
            sumCheck += mSBuff[i];
            ++i;
            temp = (int) (iter->second * 100);
            if (temp > 0) {
                mSBuff[i] = temp % 0x0100;
                sumCheck += mSBuff[i];
                ++i;
                mSBuff[i] = temp / 0x0100;

            } else {
                temp = -temp;
                mSBuff[i] = temp % 0x0100;
                sumCheck += mSBuff[i];
                ++i;
                mSBuff[i] = (0x10 + temp / 0x0100);
            }
            sumCheck += mSBuff[i];
            ++i;
        }
        mSBuff[bufSize - 1] = sumCheck;
        return bufSize;
    }

    /**
     *
     * @param p_high high bits value
     * @param p_low low bits value
     * @return 
     */
    float BioCom::calcValue(unsigned char p_high, unsigned char p_low) {
        float res = 0;
        res += p_low;
        if (p_high > 64) {
            res = (res + (p_high - 0x10)*0x0100) / 100;
            res = -res;
        } else {
            res = (res + p_high * 0x0100) / 100;
        }
        return res;
    }

     float BioCom::calcFRSValue(unsigned char p_high, unsigned char p_low){
           float res = 0;
        res += p_low;

            res = (res + p_high * 0x0100) / 100;
        return res;
     }
    void BioCom::printJointsCmd() {
        for (BCJoints::const_iterator iter = mJointsCmd.begin();
                iter != mJointsCmd.end();
                ++iter) {
            printf("JointsCmd\tid: %d, value: %f\n", iter->first, iter->second);
        }
    }

    void BioCom::printJointsInfo() {
        for (BCJoints::const_iterator iter = mJointsInfo.begin();
                iter != mJointsInfo.end();
                ++iter) {
            printf("JointsInfo\tid: %d\tvalue: %f\n", iter->first, iter->second);
        }
        if (mGRYInfo.flag != false) {
            printf("GRYInfo x: %f\ty: %f\tz: %f\n", mGRYInfo.x, mGRYInfo.y, mGRYInfo.z);
        }
        if (mRFRSInfo.flag != false) {
            printf("mRFRSInfo lu: %f\tru: %f\trd: %fld: %f\n",
                    mRFRSInfo.lu, mRFRSInfo.ru, mRFRSInfo.rd, mRFRSInfo.ld);
        }
        if (mLFRSInfo.flag != false) {
            printf("mLFRSInfo lu: %f\tru: %f\trd: %fld: %f\n",
                    mLFRSInfo.lu, mLFRSInfo.ru, mLFRSInfo.rd, mLFRSInfo.ld);
        }
    }
    
    
}
