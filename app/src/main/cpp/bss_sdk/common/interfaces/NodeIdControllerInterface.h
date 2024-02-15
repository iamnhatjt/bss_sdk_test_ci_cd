//
// Created by vnbk on 09/06/2023.
//

#ifndef NODEIDINTERFACE_H
#define NODEIDINTERFACE_H

#include <memory>

class NodeIdControllerInterface{
public:
    enum class CANBUS_NODE_ID{
        NODE_ID_LOW,
        NODE_ID_HIGH
    };
    virtual int32_t setNodeId(const uint32_t& _node_id, CANBUS_NODE_ID _up) = 0;

    virtual int32_t getNodeId(const uint32_t& _node_id) = 0;
};

#endif //NODEIDINTERFACE_H
