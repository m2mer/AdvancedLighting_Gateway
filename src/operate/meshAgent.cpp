/*
 Description: mesh agent
 
 Author: Xuesong
 
 Date: 2018-04-26
 
 */

#include <ArduinoJson.h>

#include "meshAgent.h"
#include "meshCommandPacket.h"
#include "uartProtocolPacket.h"


template<typename T>
ListNode<T>* advLinkedList<T>::getNodePtr(int index)
{
    return this->getNode(index);    //must this->
}

meshAgent::meshAgent()
{
    _init();
}
meshAgent::meshAgent(byte *mac):smartDevice(mac)
{
    _init();      
}

void meshAgent::_init()
{
    setDeviceType(SMART_DEVICE_TYPE_MESH_GATEWAY, SMART_SERVICE_DEFAULT); 
    memset(_aggBuff, 0, sizeof(AGGREGATION_UNIT)*8);
}

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
    byte bcMac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    /* all */
    if(strncmp((const char*)bcMac, (const char*)mac, 6) == 0)
    {
        *devAddr = 0xFFFF;
        return RET_OK;
    }
    /* group, 0xFF01 */
    else if(strncmp((const char*)(bcMac+1), (const char*)(mac+1), 5) == 0)
    {
        *devAddr = (0xFF<<8) + (mac[0]); 
        return RET_OK;
    }
    /* uni node */
    else
    {
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
    byte meshCmd[12] = {0};
    UART_PROTOCOL_DATA protData;
    MESH_DEVICE_COMMAND_DATA *cmd;
    byte* debug = NULL;
    uint16_t devAddr = 0;

    if(!data.success()){
        return RET_ERROR;
    }
    
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];
    if(uuid == NULL || action == NULL || value == NULL)
    {
        DEBUG_MESH.printf("%s, parameter error!\n", __FUNCTION__);
        return RET_ERROR;
    }

    //DEBUG_MESH.printf("%s, uuid is %s, action is %s, value is %s\n", __FUNCTION__, uuid, action, value);

    if(strcmp(action, "mesh_agent") != 0)
        return RET_ERROR;

    _getMeshCommandBinary(value, meshCmd);
    packet = (MESH_DEVICE_GET_STATUS*)meshCmd;

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_OPERATE_MESH_AGENT;
    cmd = (MESH_DEVICE_COMMAND_DATA*) &protData.protPayload.meshData; 
    cmd->meshCmd = packet->command;
    memcpy(cmd->cmdPara.getStatus.mac, packet->mac, 6);
    if(getMeshNodeDevAddr(packet->mac, &devAddr) == RET_OK)
    {
        cmd->devAddr = devAddr;
        //DEBUG_MESH.printf("get device address: 0x%04x\n", devAddr);
    }
    else
    {
        cmd->devAddr = 0xFFFF;
        //DEBUG_MESH.printf("node not exist, set 0xFFFF\n");
    }

    debug = (byte*) cmd;
    //DEBUG_MESH.printf("mesh data: ");
    for(int i=0; i<12; i++)
    {      
        //DEBUG_MESH.printf("%02x ", debug[i]);
    }
    //DEBUG_MESH.println();

    _deviceMp->sendUartProtocolData((byte*)&protData);        

    return RET_OK;
}

int meshAgent::getGroupStatus(byte* payload, unsigned int length)
{
    //TBD    
}

int meshAgent::deviceDelete(byte* payload, unsigned int length)
{
    char msg[256] = {0};
    char uuid[12] = {0};

    //TBD 

    /* package into mqtt message and send */
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, "\"device_deleted\"}");

    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATE_NOTIFY), msg);
    DEBUG_MESH.printf("send state notify to cloud\n");    
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
    const char* value = data["value"];
    if(uuid == NULL || action == NULL || value == NULL)
    {
        DEBUG_MESH.printf("%s, parameter error!\n", __FUNCTION__);
        return RET_ERROR;
    }

    //DEBUG_MESH.printf("%s, uuid is %s, action is %s, value is %s\n", __FUNCTION__, uuid, action, value);

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
        cmd->devAddr = devAddr;
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

    DEBUG_MESH.printf("mac %02x%02x%02x%02x%02x%02x not found\n", packet->mac[0], packet->mac[1], packet->mac[2],
        packet->mac[3], packet->mac[4], packet->mac[5]);
    return RET_ERROR;
}

int meshAgent::registrationNotify(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return RET_ERROR;
    }

    const char* uuid = data["deviceId"];
    const char* userId = data["userId"];
    if(uuid == NULL || userId == NULL)
    {
        DEBUG_MESH.printf("%s, parameter error!\n", __FUNCTION__);
        return RET_ERROR;
    }

    DEBUG_MESH.printf("%s, deviceId %s, userId %s, ok\n", __FUNCTION__, uuid, userId);

    byte mac[6] = {0};
    _getMeshCommandBinary(uuid, mac);
    /* registration notify for gateway */
    if(strncmp((const char*)mac, (const char*)_mac, 6) == 0)
    {
        DEBUG_MESH.printf("gateway registration notified\n");
        setUserId(userId);
        setMQTTDynTopic();
        setRegistered(1);

        _deviceMp->mqttUnsubscribe(_topicRegNoti);
        _deviceMp->mqttSubscribe(_topicGetSt);
        _deviceMp->mqttSubscribe(_topicDevOp);
        _deviceMp->mqttSubscribe(_topicGetGrpSt);
    }
    /* registration notify for nodes */
    else
    {
        int cnt = _meshNodeList.size();
        byte nodeMac[6] = {0};
        for(int i=0; i<cnt; i++)
        {
            ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
            node->data.getMacAddress(nodeMac);
            if(strncmp((const char*)mac, (const char*)nodeMac, 6) == 0)
            {
                DEBUG_MESH.printf("node registration notified\n");
                node->data.setRegistered(1);
            }
        }
    }

}

/* 
 * interface to receive message of mesh node from peer uart
 * 
 */
void meshAgent::receiveUARTmsg(byte *buf, int len)
{
    MESH_DEVICE_COMMAND_DATA *cmdData = (MESH_DEVICE_COMMAND_DATA*) buf;
    uint8_t meshCmd = 0;
    uint16_t nodeAddr = 0;

    if(!buf)
        return;

    DEBUG_MESH.printf("%s, receive mesh data:", __FUNCTION__);
    for(int i=0; i<len; i++) {
        DEBUG_MESH.printf("%02x ", buf[i]);
    }
    DEBUG_MESH.println();

    meshCmd = cmdData->meshCmd;
    nodeAddr = cmdData->devAddr;

    if(meshCmd == LGT_CMD_ADVLIGHT_STATUS_UPDT)
        recvStatusUpdate(nodeAddr, (byte*)&cmdData->cmdPara);
    else if(meshCmd == LGT_CMD_ADVLIGHT_OVERALL_STATUS)
        recvOverallStatus(nodeAddr, (byte*)&cmdData->cmdPara);
    else if(meshCmd == LGT_CMD_ADVLIGHT_GROUP_STATUS)
        recvGroupStatus((byte*)&cmdData->cmdPara);
    else if(meshCmd == LGT_CMD_ADVLIGHT_RESET_FACTORY)
        recvResetFactory(nodeAddr, (byte*)&cmdData->cmdPara);  
    else if(meshCmd == LGT_CMD_ADVLIGHT_PAIRED_NOTIFY)
        recvPairedNotify(nodeAddr, (byte*)&cmdData->cmdPara);      
}

/* 
 * handle status_update of mesh node
 *
 */
void meshAgent::recvStatusUpdate(uint16_t nodeAddr, byte *buf)
{
    int cnt = _meshNodeList.size();
    MESH_COMMAND_STATUS_UPDATE *update = (MESH_COMMAND_STATUS_UPDATE*) buf;
    uint16_t devAddr = nodeAddr;
    MESH_DEVICE_STATUS_UPDATE stPkt;
    char stMsg[256] = {0};

    DEBUG_MESH.printf("%s, devAddr 0x%04x\n", __FUNCTION__, devAddr);

    if(devAddr == 0)
    {
        DEBUG_MESH.printf("invalid devAddr\n");
        return;
    }

    for(int i=0; i<cnt; i++)
    {
        ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
        if(devAddr == node->data.getDevAddr())
        {
            DEBUG_MESH.printf("found node\n");

            memset(&stPkt, 0, sizeof(MESH_DEVICE_STATUS_UPDATE));
            stPkt.command = LGT_CMD_ADVLIGHT_STATUS_UPDT;
            node->data.getMacAddress(stPkt.mac);
            stPkt.sequence = update->sequence;
            stPkt.funcType = update->funcType;
            stPkt.status = update->funcPara;
            if(stPkt.funcType == MESH_DEVICE_FUNCTION_OFFLINE)
                node->data.clearStatus();

            if(node->data.checkStatusUpdateSeq(update->sequence) == RET_OK)
            {
                /* package into mqtt message and send */
                _packageMeshAgentMsg((char*)&stPkt, sizeof(MESH_DEVICE_STATUS_UPDATE), stMsg);
                _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATUS_UPDATE), stMsg);
                DEBUG_MESH.printf("send update status to cloud\n");
            }
        }
    }
}

/* 
 * handle overall_status of mesh node
 *
 */
void meshAgent::recvOverallStatus(uint16_t nodeAddr, byte *buf)
{
    int cnt = _meshNodeList.size();
    OVERALL_STATUS_AGGREGATION stAgg;
    MESH_COMMAND_OVERALL_STATUS *status = (MESH_COMMAND_OVERALL_STATUS*) buf;
    uint16_t devAddr = nodeAddr;
    meshNode *nodeP = NULL;
    MESH_DEVICE_OVERALL_STATUS stPkt;
    char stMsg[256] = {0};

    DEBUG_MESH.printf("%s, devAddr 0x%04x\n", __FUNCTION__, devAddr);

    if(devAddr == 0)
    {
        DEBUG_MESH.printf("invalid devAddr\n");
        return;
    }
#if 1
    for(int i=0; i<cnt; i++)
    {
        ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
        if(devAddr == node->data.getDevAddr())
        {
            uint8_t aggBuffId = node->data.getAggBuffId();
            DEBUG_MESH.printf("found node, aggBuffId %d,\n", aggBuffId);
            #if 0
            /* all status segments ready */
            if(node->data.aggregateStatus(buf, &stAgg))
            {
                memset(&stPkt, 0, sizeof(MESH_DEVICE_OVERALL_STATUS));
                stPkt.command = LGT_CMD_ADVLIGHT_OVERALL_STATUS;
                stPkt.sequence = status->sequence;
                memcpy(&stPkt.status, &stAgg.status, sizeof(MESH_NODE_OVERALL_STATUS));

                /* package into mqtt message and send */
                _packageMeshAgentMsg((char*)&stPkt, sizeof(MESH_DEVICE_OVERALL_STATUS), stMsg);
                _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_OVERALL_STATUS), stMsg);
                DEBUG_MESH.printf("overall status ready, reply\n");
            }
            #endif

            if(aggBuffId != 0xff)
            {
                if(aggregateStatus(aggBuffId, buf, &stAgg))
                {
                    node->data.setMAC(stAgg.status.mac); 
                    node->data.setDeviceType(stAgg.status.firstType, stAgg.status.secondType); 

                    memset(&stPkt, 0, sizeof(MESH_DEVICE_OVERALL_STATUS));
                    stPkt.command = LGT_CMD_ADVLIGHT_OVERALL_STATUS;
                    stPkt.sequence = status->sequence;
                    memcpy(&stPkt.status, &stAgg.status, sizeof(MESH_NODE_OVERALL_STATUS));

                    /* package into mqtt message and send */
                    _packageMeshAgentMsg((char*)&stPkt, sizeof(MESH_DEVICE_OVERALL_STATUS), stMsg);
                    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_OVERALL_STATUS), stMsg);
                    DEBUG_MESH.printf("overall status ready, reply\n");   
                    
                    _aggBuff[aggBuffId].devAddr = 0;
                    _aggBuff[aggBuffId].startTime = 0; 
                    _aggBuff[aggBuffId].agg.segmentMap = 0;
                    _aggBuff[aggBuffId].agg.sequence = 0;
                    node->data.setAggBuffId(0xff);                                    
                }
            }
            return;
        }
    }
#endif
    /* no mesh node found, add new node */
    nodeP = new meshNode(devAddr);
    DEBUG_MESH.printf("create node\n");
    if(!nodeP)
    {
        DEBUG_MESH.printf("new meshNode fail!\n");
        return;
    }
    nodeP->setDeviceManipulator(this->_deviceMp);

    for(int i=0; i<AGGREGATION_UNIT_NUM; i++)
    {
        if(_aggBuff[i].devAddr == 0)
        {
            nodeP->setAggBuffId(i);
            aggregateStatus(i, buf, &stAgg);
            _aggBuff[i].devAddr = devAddr;
            _aggBuff[i].startTime = millis();            
            break;
        }
    }

    /* must be at last */
    this->_meshNodeList.add(*nodeP);

    /* strange memory issue, member variable of node can't be set */
#if 0
    //nodeP->aggregateStatus(buf, &stAgg);
    for(int i=0; i<8; i++)
    {
        if(_aggBuff[i].devAddr == 0)
        {
            nodeP->setAggBuffId(i);
            aggregateStatus(i, buf, &stAgg);
            _aggBuff[i].devAddr = devAddr;
            _aggBuff[i].startTime = millis()>>16;
            break;        
        }
    }
#endif

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

/* 
 * handle reset_factory notify from mesh node
 *
 */
void meshAgent::recvResetFactory(uint16_t nodeAddr, byte *buf)
{
    int cnt = _meshNodeList.size();
    MESH_COMMAND_RESET_FACTORY *notify = (MESH_COMMAND_RESET_FACTORY*) buf;
    uint16_t devAddr = nodeAddr;
    char msg[256] = {0};
    char uuid[12] = {0};
    byte nodeMac[6];

    DEBUG_MESH.printf("%s, devAddr 0x%04x\n", __FUNCTION__, devAddr);

    if(devAddr == 0)
    {
        DEBUG_MESH.printf("invalid devAddr\n");
        return;
    }

    for(int i=0; i<cnt; i++)
    {
        ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
        if(devAddr == node->data.getDevAddr())
        {
            DEBUG_MESH.printf("found node\n");

            /* package into mqtt message and send */
            node->data.getMacAddress(nodeMac);
            sprintf(uuid, "%0x%0x%0x%0x%0x%0x", nodeMac[0],nodeMac[1],nodeMac[2],nodeMac[3],nodeMac[4],nodeMac[5]);
            strcat(msg, "{\"UUID\":\"");
            strncat(msg, uuid, 12);
            strcat(msg, "\",\"attribute\":");
            if(notify->flag == DEVICE_RESET_SOFTWARE_DELETED)
                strcat(msg, "\"device_deleted\"}");
            else
                strcat(msg, "\"hardware_reset\"}");

            _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATE_NOTIFY), msg);
            DEBUG_MESH.printf("send state notify to cloud, %s\n", msg);

            //TBD, delete node?
            _meshNodeList.remove(i);
        }
    }
}

void meshAgent::recvPairedNotify(uint16_t nodeAddr, byte *buf)
{
    int cnt = _meshNodeList.size();
    MESH_COMMAND_PAIRED_NOTIFY *notify = (MESH_COMMAND_PAIRED_NOTIFY*) buf;
    meshNode *nodeP = NULL;
    MESH_DEVICE_OVERALL_STATUS stPkt;
    char stMsg[256] = {0};
    int id = 0;

    DEBUG_MESH.printf("%s, devAddr 0x%04x\n", __FUNCTION__, nodeAddr);

    if(nodeAddr == 0)
    {
        DEBUG_MESH.printf("invalid devAddr\n");
        return;
    }

    if(notify->firstType == SMART_DEVICE_TYPE_MESH_GATEWAY)
    {
        memcpy(_meshMAC, notify->mac, 6);
        /* gateway register to cloud */
        //this->deviceRegister();    //wait wifi to be configured
    }
    /* depracted for node */
    else
    {
        for(id; id<cnt; id++)
        {
            ListNode<meshNode> *node = _meshNodeList.getNodePtr(id);
            if(nodeAddr == node->data.getDevAddr())
            {
                nodeP = &node->data;
                break;
            }
        }
        if(id >= cnt)
        {
            /* no mesh node found, add new node */
            nodeP = new meshNode(nodeAddr);
            if(!nodeP)
            {
                DEBUG_MESH.printf("new meshNode fail!\n");
                return;
            }
            nodeP->setDeviceType(notify->firstType, notify->secondType);  
            this->_meshNodeList.add(*nodeP);
        }

        /* mesh node register to cloud */
        nodeP->deviceRegister(_mac);
    }

}

/* 
 * deviceRegister
 * register gateway to cloud, contain wifi device and mesh agent
 * 
 */
void meshAgent::deviceRegister() 
{
    uint8 mac[6];
    char wifi_mac[13] = {0};
    char mesh_mac[13] = {0};
    char bssid[32] = {0};
    char type[9] = {0};
    char msg[256] = {0};

    /* subscribe registration_notify first */
    _deviceMp->mqttSubscribe(_topicRegNoti);
    _registerTime = millis();

    getMacAddress(mac);
    WiFi.BSSIDstr().toCharArray(bssid, 32, 0);
    DEBUG_DEVICE.printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    DEBUG_DEVICE.printf("get bssid %s\n", bssid);

    sprintf(type, "%04x%04x", this->_type.firstType, this->_type.secondType);
    sprintf(wifi_mac, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    sprintf(mesh_mac, "%02x%02x%02x%02x%02x%02x", _meshMAC[0],_meshMAC[1],_meshMAC[2],_meshMAC[3],_meshMAC[4],_meshMAC[5]);

    strcat(msg, "{\"type\":\"");
    strcat(msg, type);
    strcat(msg, "\",\"vendor\":\"AISmart\",\"MAC\":\"");
    strcat(msg, wifi_mac);
    strcat(msg, "\",\"BSSID\":\"");
    strcat(msg, bssid);
    strcat(msg, "\",\"meshId\":\"");
    strcat(msg, mesh_mac);
    strcat(msg, "\"}");

    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_DEVICE_REGISTER), msg);
    DEBUG_DEVICE.printf("pub device register %s\n", msg);
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
void meshAgent::_packageMeshAgentMsg(char *buf, int len, char *msg) 
{
    char uuid[12] = {0};
    char buf_str[128] = {0};
    char *ptr = buf;

    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
    for(int i=0; i<len; i++)
    {
        sprintf(&buf_str[2*i], "%02x", *(ptr++));
    }    

    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, "mesh_agent");
    strcat(msg, "\",\"value\":\"");
    //strcat(msg, (const char*)buf);
    strcat(msg, (const char*)buf_str);
    strcat(msg, "\"}");

    DEBUG_DEVICE.printf("gen msg: %s\n", msg);
}

boolean meshAgent::aggregateStatus(int buffId, byte *buf, OVERALL_STATUS_AGGREGATION *stAgg)
{
    uint8_t sequence = 0;
    uint8_t segment = 0;
    MESH_COMMAND_OVERALL_STATUS *status = (MESH_COMMAND_OVERALL_STATUS*) buf;
    MESH_COMMAND_OVERALL_STATUS_I *statusI = (MESH_COMMAND_OVERALL_STATUS_I*) (buf+2);
    MESH_COMMAND_OVERALL_STATUS_II *statusII = (MESH_COMMAND_OVERALL_STATUS_II*) (buf+2);
    OVERALL_STATUS_AGGREGATION *agg = &(_aggBuff[buffId].agg);

    if(!buf)
    {
        DEBUG_MESH.println("buf is null");
        return false;
    }

    sequence = status->sequence;
    segment = status->segment;
    DEBUG_MESH.printf("in seq %d, _seq %d, segMap 0x%0x\n", sequence,  agg->sequence, agg->segmentMap);

    if(sequence == agg->sequence && agg->segmentMap == 0x03)  //repeated packet
        return false;
    if(sequence != agg->sequence)
    {
        agg->segmentMap &= 0x0e;    //only clear segment0, to speed up overall_status       
        agg->sequence = sequence;
    }

    switch(segment)
    {
        case MESH_OVERALL_STATUS_I:
            agg->segmentMap |= 0x01;         
            agg->status.group = statusII->group;
            agg->status.onoff = statusII->onoff;
            agg->status.lightness = statusII->lightness;
            agg->status.mode = statusII->mode;
            if(agg->status.mode == SMART_LIGHT_TYPE_WC)
                agg->status.temperature = statusII->rgbcw.temperature;
            else if(agg->status.mode == SMART_LIGHT_TYPE_RGB)
            {
                agg->status.color.h = statusII->rgbcw.color.h;
                agg->status.color.s = statusII->rgbcw.color.s;
                agg->status.color.v = statusII->rgbcw.color.v; 
            }   
            break;
        case MESH_OVERALL_STATUS_II:
            agg->segmentMap |= 0x02;     
            memcpy(agg->status.mac, statusI->mac, 6);
            agg->status.firstType = statusI->firstType;
            agg->status.secondType = statusI->secondType;

            /* a new paired node, register to cloud */
            if(sequence == 0)
            {
                DEBUG_MESH.printf("new node, register to cloud\n");
                this->deviceRegister();
            }                                                               
            break;
    }

    DEBUG_MESH.printf("%s, segmentMap %d\n", __FUNCTION__, agg->segmentMap);
    if(agg->segmentMap == 0x03)  //0x0f
    {
        memcpy(stAgg, agg, sizeof(OVERALL_STATUS_AGGREGATION));
        return true;
    }
    else
        return false;

}

int meshAgent::hardwareReset()
{
    char msg[256] = {0};
    char uuid[12] = {0};

    //TBD 

    /* package into mqtt message and send */
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, "\"hardware_reset\"}");

    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATE_NOTIFY), msg);
    DEBUG_MESH.printf("send state notify to cloud\n");   

}

void meshAgent::setNetworkConfiged(int flag)
{
    _networkConfiged = flag;
}

int meshAgent::getNetworkConfged()
{
    return _networkConfiged;
}

void meshAgent::notifyMeshAgent(int networkConfiged)
{
    UART_PROTOCOL_DATA protData;

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_NOTIFY_MESH_AGENT;
    protData.protPayload.meshAGdata.networkConfiged = networkConfiged;

    _deviceMp->sendUartProtocolData((byte*)&protData);  
    DEBUG_MESH.printf("send network config state %d to meshAgent\n", networkConfiged);
}

void meshAgent::metaInfoManage()
{
    MESH_NODE_METADATA _nodeMeta[MESH_NODE_NUM_MAX];
    int storeId = 0;
    uint32_t now = millis();
    int needStore = 0;

    if((now - _metaInfoTime) < 10000)
        return; 

    _metaInfoTime = now;

    /* gateway need register again */
    if(_registered == 0 && (now-_registerTime)> 10000)
    {
        deviceRegister();
        _registerTime = now;
    }
    /* gateway registered */
    else if(_registered == 1 && _registerTime != 0)
        needStore = 1;

    int cnt = _meshNodeList.size();
    meshNode *node = NULL;
    byte nodeMac[6] = {0};
    for(int i=0; i<cnt; i++)
    {
        ListNode<meshNode> *nodeList = _meshNodeList.getNodePtr(i);
        node = &nodeList->data;
        node->getMacAddress(nodeMac);

        /* node need register again */
        if(nodeMac[0] != 0 && node->getRegistered() == 0 && (now-node->getRegisterTime())> 10000)
        {
            node->deviceRegister(_mac);
            node->setRegisterTime(now);
        }
        /* node registered */
        else if(node->getRegistered() == 1 && node->getRegisterTime() != 0)
            needStore = 1;

        _nodeMeta[storeId].deviceAddress = node->getDevAddr();
        memcpy(_nodeMeta[storeId].mac, nodeMac, 6);
        _nodeMeta[storeId].registered = node->getRegistered();
        storeId++;

        DEBUG_MESH.printf("\n--devAddr: 0x%04x\n", node->getDevAddr());
        DEBUG_MESH.printf("--mac: 0x%02x%02x%02x%02x%02x%02x\n", nodeMac[0],
                            nodeMac[1],nodeMac[2],nodeMac[3],nodeMac[4],nodeMac[5]);
    }

    if(needStore)
    {
        //TBD, restore to flash
    }
}

void meshAgent::aggBuffManage()
{
    int cnt = _meshNodeList.size();
    uint32_t now = millis();
    int needStore = 0;

    if((now - _aggBuffMgTime) < 1000)
        return; 

    _aggBuffMgTime = now;

    for(int i=0; i<AGGREGATION_UNIT_NUM; i++)
    {
        if(_aggBuff[i].devAddr != 0 && _aggBuff[i].startTime != 0
            && (now - _aggBuff[i].startTime > 1000) && _aggBuff[i].agg.segmentMap != 0x03)
            {
                DEBUG_MESH.printf("node 0x%02x overall_status exceeds 1s, quit aggBuff\n", _aggBuff[i].devAddr);                 
                _aggBuff[i].devAddr = 0;
                _aggBuff[i].startTime = 0;
                _aggBuff[i].agg.segmentMap = 0;
                _aggBuff[i].agg.sequence = 0;

                for(int i=0; i<cnt; i++)
                {
                    ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
                    if(node->data.getDevAddr() == _aggBuff[i].devAddr) 
                    {
                        node->data.setAggBuffId(0xff);
                        break;
                    }               
                }         
            }

    }
}

void meshAgent::loop()
{

    aggBuffManage();
    heartbeat();
    metaInfoManage();
}
