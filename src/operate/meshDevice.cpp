/*
 Description: mesh device
 
 Author: Xuesong
 
 Date: 2018-04-26
 
 */

#include <ArduinoJson.h>

#include "meshDevice.h"
#include "meshCommandPacket.h"
#include "uartProtocolPacket.h"


#define DEBUG_MESH Serial1


boolean meshAgent::isMeshNodeExist(byte *mac)
{
    int cnt = _meshNodeList.size();
    meshNode node;
    byte nodeMac[6] = {0};

    for(int i=0; i<cnt; i++)
    {
        node = _meshNodeList.get(i);
        node.getMacAddress(nodeMac);
        if(strncmp((const char*)nodeMac, (const char*)mac, 6) == 0)
            return true;
    }

    return false;
}

int meshAgent::addMeshNode(byte *mac, uint16_t devAddr)
{
    meshNode *node = new meshNode(mac, devAddr);

    if(this->_meshNodeList.add(*node))
        return 0;
    else
        return -1;
}

int meshAgent::delMeshNode(byte *mac)
{
    int cnt = _meshNodeList.size();
    meshNode node;
    byte nodeMac[6] = {0};

    for(int i=0; i<cnt; i++)
    {
        node = _meshNodeList.get(i);
        node.getMacAddress(nodeMac);
        if(strncmp((const char*)nodeMac, (const char*)mac, 6) == 0)
        {
            _meshNodeList.remove(i);
            return 0;
        }
    }

    return -1;
}


int meshAgent::getMeshNodeDevAddr(byte *mac, uint16_t *devAddr)
{
    int cnt = _meshNodeList.size();
    meshNode node;
    byte nodeMac[6] = {0};

    for(int i=0; i<cnt; i++)
    {
        node = _meshNodeList.get(i);
        node.getMacAddress(nodeMac);
        if(strncmp((const char*)nodeMac, (const char*)mac, 6) == 0)
        {
            *devAddr = node.getDevAddr();
            return RET_OK;
        }
    }

    return RET_ERROR;
}

/* 
 * device_operate
 * convert MESH_DEVICE_OPERATION to MESH_DEVICE_COMMAND_DATA,
 * send to peer mesh agent through uart
 * 
 */
int meshAgent::operateDevice(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    MESH_DEVICE_OPERATION *packet = NULL;
    byte meshCmd[16] = {0};
    uint16_t devAddr = 0;
    MESH_DEVICE_COMMAND_DATA *cmd;
    UART_PROTOCOL_DATA protData;
    byte* debug = NULL;

    if(!data.success()){
        return RET_ERROR;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];;

    DEBUG_MESH.printf("%s, uuid is %s, action is %s, value is %s\n", __FUNCTION__, uuid, action, value);

    if(strcmp(action, "mesh_agent") != 0)
        return RET_ERROR;
    
    _getMeshCommandBinary(value, meshCmd);

    packet = (MESH_DEVICE_OPERATION*)meshCmd;
    if(getMeshNodeDevAddr(packet->mac, &devAddr) == RET_OK)
    {
        memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
        protData.protType = PROTOCOL_TYPE_OPERATE_MESH_AGENT;
        cmd = &protData.protPayload.meshData; 
        cmd->meshCmd = packet->command;
        cmd->cmdPara.operation.devAddr = devAddr;
        cmd->cmdPara.operation.funcType = packet->funcType;
        memcpy(cmd->cmdPara.operation.funcPara, packet->funcPara, 5);

        debug = (byte*) cmd;
        for(int i=0; i<12; i++)
        {      
            DEBUG_MESH.printf("%d ", debug[i]);
        }

        _deviceMp->sendUartProtocolData((byte*)&protData);        

        return RET_OK;
    }

    return RET_ERROR;
}

/* 
 * get_status
 * convert MESH_DEVICE_GET_STATUS to MESH_DEVICE_COMMAND_DATA,
 * send to peer mesh agent through uart
 * 
 */
int meshAgent::getOverallStatus(byte* payload, unsigned int length) 
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    MESH_DEVICE_GET_STATUS *packet = NULL;
    byte meshCmd[16] = {0};
    UART_PROTOCOL_DATA protData;
    MESH_DEVICE_COMMAND_DATA *cmd;
    byte* debug = NULL;

    if(!data.success()){
        return RET_ERROR;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];

    DEBUG_MESH.printf("%s, uuid is %s, action is %s, value is %s\n", __FUNCTION__, uuid, action, value);

    if(strcmp(action, "mesh_agent") != 0)
        return RET_ERROR;

    _getMeshCommandBinary(value, meshCmd);
    packet = (MESH_DEVICE_GET_STATUS*)meshCmd;

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_GET_MESH_AGENT;
    cmd = (MESH_DEVICE_COMMAND_DATA*) &protData.protPayload.meshData; 
    cmd->meshCmd = packet->command;
    memcpy(cmd->cmdPara.getStatus.mac, packet->mac, 6);

    debug = (byte*) cmd;
    for(int i=0; i<12; i++)
    {      
        DEBUG_MESH.printf("%d ", debug[i]);
    }

    _deviceMp->sendUartProtocolData((byte*)&protData);        

    return RET_OK;
}

/* 
 * interface to receive message of mesh node from peer uart
 * 
 */
void meshAgent::receiveUARTmsg(byte *buf, int len)
{
    MESH_DEVICE_COMMAND_DATA *cmdData = (MESH_DEVICE_COMMAND_DATA*) buf;
    uint8_t meshCmd = 0;

    if(!buf)
        return;

    DEBUG_MESH.printf("%s, receive mesh data:", __FUNCTION__);
    for(int i=0; i<len; i++) {
        DEBUG_MESH.printf("%02x ", buf[i]);
    }

    meshCmd = cmdData->meshCmd;

    if(meshCmd == LGT_CMD_ADVLIGHT_STATUS_UPDT)
        recvStatusUpdate((byte*)&cmdData->cmdPara);
    else if(meshCmd == LGT_CMD_ADVLIGHT_OVERALL_STATUS)
        recvOverallStatus((byte*)&cmdData->cmdPara);
    else if(meshCmd == LGT_CMD_ADVLIGHT_GROUP_STATUS)
        recvGroupStatus((byte*)&cmdData->cmdPara);
}

/* 
 * handle status_update of mesh node
 *
 */
void meshAgent::recvStatusUpdate(byte *buf)
{
    MESH_COMMAND_STATUS_UPDATE *status = (MESH_COMMAND_STATUS_UPDATE*) buf;

    //TBD
}

/* 
 * handle overall_status of mesh node
 *
 */
void meshAgent::recvOverallStatus(byte *buf)
{
    int cnt = _meshNodeList.size();
    meshNode node;
    uint16_t devAddr = 0;
    OVERALL_STATUS_AGGREGATION stAgg;
    MESH_COMMAND_OVERALL_STATUS *status = (MESH_COMMAND_OVERALL_STATUS*) buf;
    MESH_DEVICE_OVERALL_STATUS stPkt;
    char stMsg[256] = {0};

    devAddr = status->status.statusI.deviceAddr;
    for(int i=0; i<cnt; i++)
    {
        node = _meshNodeList.get(i);
        if(devAddr == node.getDevAddr())
        {
            /* all status segments ready */
            if(node.aggregateStatus(buf, &stAgg))
            {
                memset(&stPkt, 0, sizeof(MESH_DEVICE_OVERALL_STATUS));
                stPkt.command = LGT_CMD_ADVLIGHT_OVERALL_STATUS;
                stPkt.sequence = status->sequence;
                memcpy(&stPkt.status, &stAgg.status, sizeof(MESH_NODE_OVERALL_STATUS));

                /* package into mqtt message and send */
                _packageMeshAgentMsg((char*)&stPkt, stMsg);
                _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_OVERALL_STATUS), stMsg);
                
                return;
            }
        }
    }

    /* no mesh node found, add new node */
    meshNode *nodeP = new meshNode(devAddr);
    if(this->_meshNodeList.add(*nodeP))
    {
        nodeP->aggregateStatus(buf, &stAgg);      
    }

}

/* 
 * handle group_status_update of mesh node
 *
 */
void meshAgent::recvGroupStatus(byte *buf)
{
    MESH_COMMAND_OVERALL_STATUS *status = (MESH_COMMAND_OVERALL_STATUS*) buf;

    //TBD
}


int meshAgent::_atoi(char a)
{
    if((a-'a' >= 0) && (a-'f' <= 0))
        return (a-'a') + 10;
    else if((a-'A' >= 0) && (a-'F' <= 0))
        return (a-'A')  + 10;
    else if((a-'0' >= 0) && (a-'9' <= 0))
        return a-'0';
    else
        return 0;
}

void meshAgent::_getMeshCommandBinary(const char *buf, byte *bin)
{

    while(*buf)
    {
        *(bin++) = _atoi(buf[0])*16 + _atoi(buf[1]);

        buf+=2;
    }

}


/*
 * package passthrough Mesh agent binary in json filed "mesh_agent"
*/
void meshAgent::_packageMeshAgentMsg(char *buf, char *msg) 
{
    byte mac[6] = {0};
    char uuid[12] = {0};

    DEBUG_DEVICE.printf("%s\n", __FUNCTION__);

    getMacAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, "mesh_agent");
    strcat(msg, "\",\"value\":\"");
    strcat(msg, (const char*)buf);
    strcat(msg, "\"}");
}


/*
 * meshNode functions
 *
*/
meshNode::meshNode(uint16_t devAddr)
{
    _devAddr = devAddr;
}

meshNode::meshNode(byte *mac, uint16_t devAddr):smartDevice(mac)
{
    _devAddr = devAddr;
}

uint16_t meshNode::getDevAddr()
{
    return _devAddr;
}

boolean meshNode::aggregateStatus(byte *buf, OVERALL_STATUS_AGGREGATION *stAgg)
{
    uint8_t sequence = 0;
    uint8_t segment = 0;
    MESH_COMMAND_OVERALL_STATUS_I *statusI = (MESH_COMMAND_OVERALL_STATUS_I*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_II *statusII = (MESH_COMMAND_OVERALL_STATUS_II*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_III *statusIII = (MESH_COMMAND_OVERALL_STATUS_III*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_IV *statusIV = (MESH_COMMAND_OVERALL_STATUS_IV*) (buf+2);

    if(!buf)
        return false;

    sequence = buf[0];
    segment = buf[1];

    if(sequence < _stAgg.sequence)
        return false;                    // old status, abandon
    else if(sequence > _stAgg.sequence)
        _stAgg.segmentMap = 0;           // new status, clear old map
    
    switch(segment)
    {
        case MESH_OVERALL_STATUS_I:
            _stAgg.segmentMap |= 0x01;
            setMAC(statusI->mac);       
            memcpy(_stAgg.status.mac, statusI->mac, 6);
            break;
        case MESH_OVERALL_STATUS_II:
            _stAgg.segmentMap |= 0x02;         
            _stAgg.status.firstType = statusII->firstType;
            _stAgg.status.secondType = statusII->secondType;
            _stAgg.status.group = statusII->group;
            _stAgg.status.onoff = statusII->onoff;
            _stAgg.status.lightness = statusII->lightness;
            _stAgg.status.mode = statusII->mode;                                                            
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

    if(_stAgg.segmentMap == 0x0f)
    {
        memcpy(stAgg, &_stAgg, sizeof(OVERALL_STATUS_AGGREGATION));
        return true;
    }
    else
        return false;

}






