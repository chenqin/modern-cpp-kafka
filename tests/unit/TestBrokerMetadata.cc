#include "kafka/BrokerMetadata.h"

#include "gtest/gtest.h"

namespace Kafka = KAFKA_API;

TEST(BrokerMetadata, Node)
{
    Kafka::BrokerMetadata::Node::Id id     = 1;
    Kafka::BrokerMetadata::Node::Host host = "127.0.0.1";
    Kafka::BrokerMetadata::Node::Port port = 9000;

    Kafka::BrokerMetadata::Node node(id, host, port);

    EXPECT_EQ(id,   node.id);
    EXPECT_EQ(host, node.host);
    EXPECT_EQ(port, node.port);
    EXPECT_EQ("127.0.0.1:9000/1", node.toString());
}

TEST(BrokerMetadata, Basic)
{
    Kafka::Topic topic("topicName");
    std::vector<Kafka::BrokerMetadata::Node> nodes = {{1, "server1", 9000}, {2, "server2", 9000}, {3, "server3", 9000}};
    int numNode      = nodes.size();
    int numPartition = numNode;

    Kafka::BrokerMetadata metadata(topic);
    metadata.setOrigNodeName(nodes[0].host);

    // Add nodes
    for (const auto& node: nodes)
    {
        metadata.addNode(node.id, node.host, node.port);
    }

    EXPECT_EQ(nodes.size(), metadata.nodes().size());

    // Add info for partitions
    for (Kafka::Partition partition = 0; partition < numPartition; ++partition)
    {
        Kafka::BrokerMetadata::PartitionInfo partitionInfo;
        partitionInfo.setLeader(nodes[partition].id);
        for (const auto& node: nodes)
        {
            partitionInfo.addReplica(node.id);
            partitionInfo.addInSyncReplica(node.id);
        }
        metadata.addPartitionInfo(partition, partitionInfo);
    }

    EXPECT_EQ(topic, metadata.topic());
    EXPECT_EQ(numPartition, metadata.partitions().size());

    for (Kafka::Partition partition = 0; partition < numPartition; ++partition)
    {
        const auto& partitionInfo = metadata.partitions().at(partition);
        EXPECT_EQ(nodes[partition].id,   partitionInfo.leader);
        EXPECT_EQ(numNode, partitionInfo.replicas.size());
        EXPECT_EQ(numNode, partitionInfo.inSyncReplicas.size());
    }

    std::string expectedMetadata = std::string("originatingNode[server1], topic[topicName], partitions{")
        + "0: leader[server1:9000/1], replicas[server1:9000/1, server2:9000/2, server3:9000/3], inSyncReplicas[server1:9000/1, server2:9000/2, server3:9000/3]; "
        + "1: leader[server2:9000/2], replicas[server1:9000/1, server2:9000/2, server3:9000/3], inSyncReplicas[server1:9000/1, server2:9000/2, server3:9000/3]; "
        + "2: leader[server3:9000/3], replicas[server1:9000/1, server2:9000/2, server3:9000/3], inSyncReplicas[server1:9000/1, server2:9000/2, server3:9000/3]}";
    EXPECT_EQ(expectedMetadata, metadata.toString());
}

TEST(BrokerMetadata, IncompleteInfo)
{
    Kafka::Topic topic("topicName");
    std::vector<Kafka::BrokerMetadata::Node> nodes = {{1, "server1", 9000}, {2, "server2", 9000}, {3, "server3", 9000}};
    int numNode      = nodes.size();
    int numPartition = numNode;

    Kafka::BrokerMetadata metadata(topic);
    metadata.setOrigNodeName(nodes[0].host);

    // Add nodes (not complete)
    metadata.addNode(nodes[0].id, nodes[0].host, nodes[0].port);

    // Add info for partitions
    for (Kafka::Partition partition = 0; partition < numPartition; ++partition)
    {
        Kafka::BrokerMetadata::PartitionInfo partitionInfo;
        partitionInfo.setLeader(nodes[partition].id);
        for (const auto& node: nodes)
        {
            partitionInfo.addReplica(node.id);
            partitionInfo.addInSyncReplica(node.id);
        }
        metadata.addPartitionInfo(partition, partitionInfo);
    }

    std::string expectedMetadata = std::string("originatingNode[server1], topic[topicName], partitions{")
        + "0: leader[server1:9000/1], replicas[server1:9000/1, -:-/2, -:-/3], inSyncReplicas[server1:9000/1, -:-/2, -:-/3]; "
        + "1: leader[-:-/2], replicas[server1:9000/1, -:-/2, -:-/3], inSyncReplicas[server1:9000/1, -:-/2, -:-/3]; "
        + "2: leader[-:-/3], replicas[server1:9000/1, -:-/2, -:-/3], inSyncReplicas[server1:9000/1, -:-/2, -:-/3]}";
    EXPECT_EQ(expectedMetadata, metadata.toString());
}

