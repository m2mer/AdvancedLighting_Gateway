/*
 Description: mesh node
 
 Author: Xuesong
 
 Date: 2018-05-25
 
 */

#include "meshNode.h"




meshNode::meshNode()
{
    init();
}

meshNode::meshNode(uint16_t devAddr)
{
    _devAddr = devAddr;
    init();
}

meshNode::meshNode(byte *mac, uint16_t devAddr):smartDevice(mac)
{
    _devAddr = devAddr;
    init();
}

void meshNode::init()
{
    memset(&_stAgg, 0, sizeof(OVERALL_STATUS_AGGREGATION));
}

uint16_t meshNode::getDevAddr()
{
    return _devAddr;
}

void meshNode::setGatewayMAC(uint8_t *mac)
{
    memcpy(_gwMAC, mac, 6);
}

void meshNode::clearStatus()
{
    memset(&this->_stAgg, 0, sizeof(OVERALL_STATUS_AGGREGATION));
    /* make sequence 0 when offline, to avoid node sequence become old after reboot */
    _stUpdSeq = 0;
}

boolean meshNode::aggregateStatus(byte *buf, OVERALL_STATUS_AGGREGATION *stAgg)
{
    uint8_t sequence = 0;
    uint8_t segment = 0;
    MESH_COMMAND_OVERALL_STATUS *status = (MESH_COMMAND_OVERALL_STATUS*) buf;
    MESH_COMMAND_OVERALL_STATUS_I *statusI = (MESH_COMMAND_OVERALL_STATUS_I*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_II *statusII = (MESH_COMMAND_OVERALL_STATUS_II*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_III *statusIII = (MESH_COMMAND_OVERALL_STATUS_III*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_IV *statusIV = (MESH_COMMAND_OVERALL_STATUS_IV*) (buf+2);

    if(!buf)
    {
        DEBUG_MESH.println("buf is null");
        return false;
    }

    sequence = status->sequence;
    segment = status->segment;
    DEBUG_MESH.printf("in seq %d, _seq %d, segMap 0x%0x\n", sequence,  _stAgg.sequence, _stAgg.segmentMap);
#if 0
    if(sequence < _stAgg.sequence && sequence != 0)  // old status, abandon
        return false;                    
    else if(sequence == _stAgg.sequence && _stAgg.segmentMap == 0x03)  //repeated packet
        return false;
    else if(sequence > _stAgg.sequence || sequence == 0)  // new status, clear old map
    {
        _stAgg.segmentMap = 0;           
        _stAgg.sequence = sequence;
    }
#endif
    if(sequence == _stAgg.sequence && _stAgg.segmentMap == 0x03)  //repeated packet
        return false;
    if(sequence != _stAgg.sequence)
    {
        _stAgg.segmentMap &= 0xe0;    //only clear segment0, to speed up overall_status       
        _stAgg.sequence = sequence;
    }

    switch(segment)
    {
        case MESH_OVERALL_STATUS_I:
            _stAgg.segmentMap |= 0x01;         
            _stAgg.status.group = statusII->group;
            _stAgg.status.onoff = statusII->onoff;
            _stAgg.status.lightness = statusII->lightness;
            _stAgg.status.mode = statusII->mode;
            if(_stAgg.status.mode == SMART_LIGHT_TYPE_WC)
                _stAgg.status.temperature = statusII->rgbcw.temperature;
            else if(_stAgg.status.mode == SMART_LIGHT_TYPE_RGB)
            {
                _stAgg.status.color.h = statusII->rgbcw.color.h;
                _stAgg.status.color.s = statusII->rgbcw.color.s;
                _stAgg.status.color.v = statusII->rgbcw.color.v; 
            }   
            break;
        case MESH_OVERALL_STATUS_II:
            _stAgg.segmentMap |= 0x02;
            setMAC(statusI->mac); 
            setDeviceType(statusI->firstType, statusI->secondType);      
            memcpy(_stAgg.status.mac, statusI->mac, 6);
            _stAgg.status.firstType = statusI->firstType;
            _stAgg.status.secondType = statusI->secondType;

            /* a new paired node, register to cloud */
            if(sequence == 0)
            {
                DEBUG_MESH.printf("new node, register to cloud\n");
                this->deviceRegister();
            }                                                               
            break;
        case MESH_OVERALL_STATUS_III:
            _stAgg.segmentMap |= 0x04; 
            _stAgg.status.temperature = statusIII->temperature;
            _stAgg.status.color.h = statusIII->color.h;
            _stAgg.status.color.s = statusIII->color.s;
            _stAgg.status.color.v = statusIII->color.v;
            break;
        case MESH_OVERALL_STATUS_IV:
            _stAgg.segmentMap |= 0x08;
            _stAgg.status.timerOn = statusIV->timerOn;
            _stAgg.status.timerOff = statusIV->timerOff;
            break;
    }

    DEBUG_MESH.printf("%s, segmentMap %d\n", __FUNCTION__, _stAgg.segmentMap);
    if(_stAgg.segmentMap == 0x03)  //0x0f
    {
        memcpy(stAgg, &_stAgg, sizeof(OVERALL_STATUS_AGGREGATION));
        return true;
    }
    else
        return false;

}

int meshNode::checkStatusUpdateSeq(uint8_t sequence)
{
    if(sequence == _stUpdSeq && sequence != 0)
    {
        DEBUG_MESH.printf("%s, sequence %d repeated\n", __FUNCTION__, sequence);
        return RET_ERROR;
    }

    _stUpdSeq = sequence;
    return RET_OK;
}

void meshNode::deviceRegister() 
{
    uint8 mac[6];
    char nd_mac[13] = {0};
    char gw_mac[13] = {0};
    char dev_addr[5] = {0};
    char bssid[32] = {0};
    char type[9] = {0};
    char msg[256] = {0};

    /* subscribe registration_notify first */
    _deviceMp->mqttSubscribe(_topicRegNoti);

    getMacAddress(mac);
    WiFi.BSSIDstr().toCharArray(bssid, 32, 0);
    DEBUG_DEVICE.printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    DEBUG_DEVICE.printf("get bssid %s\n", bssid);

    sprintf(type, "%04x%04x", this->_type.firstType, this->_type.secondType);
    sprintf(nd_mac, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    sprintf(gw_mac, "%02x%02x%02x%02x%02x%02x", _gwMAC[0],_gwMAC[1],_gwMAC[2],_gwMAC[3],_gwMAC[4],_gwMAC[5]);
    sprintf(dev_addr, "%04x", this->_devAddr);
    strcat(msg, "{\"type\":\"");
    strcat(msg, type);
    strcat(msg, "\",\"vendor\":\"AISmart\",\"MAC\":\"");
    strcat(msg, nd_mac);
    strcat(msg, "\",\"gatewayId\":\"");
    strcat(msg, gw_mac);
    strcat(msg, "\",\"devAddr\":\"");
    strcat(msg, dev_addr);
    strcat(msg, "\"}");

    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_DEVICE_REGISTER), msg);
    DEBUG_DEVICE.printf("pub device register %s\n", msg);
}

