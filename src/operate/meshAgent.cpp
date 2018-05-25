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
    setDeviceType(SMART_DEVICE_TYPE_MESH_GATEWAY, SMART_SERVICE_DEFAULT); 
}
meshAgent::meshAgent(byte *mac):smartDevice(mac)
{
    setDeviceType(SMART_DEVICE_TYPE_MESH_GATEWAY, SMART_SERVICE_DEFAULT);      
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

    //DEBUG_MESH.printf("%s, uuid is %s, action is %s, value is %s\n", __FUNCTION__, uuid, action, value);

    if(strcmp(action, "mesh_agent") != 0)
        return RET_ERROR;

    _getMeshCommandBinary(value, meshCmd);
    packet = (MESH_DEVICE_GET_STATUS*)meshCmd;

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_GET_MESH_AGENT;
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
    const char* value = data["value"];;

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

            stPkt.command = LGT_CMD_ADVLIGHT_STATUS_UPDT;
            node->data.getMacAddress(stPkt.mac);
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

    for(int i=0; i<cnt; i++)
    {
        ListNode<meshNode> *node = _meshNodeList.getNodePtr(i);
        if(devAddr == node->data.getDevAddr())
        {
            DEBUG_MESH.printf("found node\n");
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

            return;
        }
    }

    /* no mesh node found, add new node */
    nodeP = new meshNode(devAddr);
    if(!nodeP)
    {
        DEBUG_MESH.printf("new meshNode fail!\n");
        return;
    }     
    nodeP->setGatewayMAC(_mac);
    nodeP->setDeviceManipulator(this->_deviceMp);
    this->_meshNodeList.add(*nodeP);
    nodeP->aggregateStatus(buf, &stAgg);
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
            sprintf(uuid, "%0x%0x%0x%0x%0x%0x", _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5]);
            strcat(msg, "{\"UUID\":\"");
            strncat(msg, uuid, 12);
            strcat(msg, "\",\"attribute\":\"");
            if(notify->flag == DEVICE_RESET_SOFTWARE_DELETED)
                strcat(msg, "\"device_deleted\"}");
            else
                strcat(msg, "\"hardware_reset\"}");

            _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATE_NOTIFY), msg);
            DEBUG_MESH.printf("send state notify to cloud\n");

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
        this->deviceRegister();
    }
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
            nodeP->setGatewayMAC(_mac);
            nodeP->setDeviceType(notify->firstType, notify->secondType);  
            this->_meshNodeList.add(*nodeP);
        }

        /* mesh node register to cloud */
        nodeP->deviceRegister();
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


