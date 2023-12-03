#include "idl/MessagePubSubTypes.h"

#include <chrono>
#include <thread>
#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

using namespace eprosima::fastdds::dds;

class HelloWorldPublisher
{
private:
    Message hello_;
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;

    class PubListener : public DataWriterListener
    {
    public:
        std::atomic_int matched_;

        PubListener() : matched_(0) { }
        ~PubListener() override { }

        void on_publication_matched(DataWriter*, const PublicationMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
            }
        }
    } listener_;

public:

    HelloWorldPublisher()
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new MessagePubSubType())
    {
    }

    virtual ~HelloWorldPublisher()
    {
        if (writer_ != nullptr) { publisher_->delete_datawriter(writer_); }
        if (publisher_ != nullptr) { participant_->delete_publisher(publisher_); }
        if (topic_ != nullptr) { participant_->delete_topic(topic_); }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the publisher
    bool init()
    {
        hello_.data()[0] = 0;

        DomainParticipantQos participantQos;
        participantQos.name("Participant_publisher");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the publications Topic
        topic_ = participant_->create_topic("MessageTopic", "Message", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        // Create the Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            return false;
        }

        // Create the DataWriter
        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

        if (writer_ == nullptr)
        {
            return false;
        }
        return true;
    }

    //!Send a publication
    bool publish(uint payload)
    {
        if (listener_.matched_ > 0)
        {
            hello_.data()[0] = static_cast<int>(payload);
            writer_->write(&hello_);
            return true;
        }
        return false;
    }

    //!Run the Publisher
    void on_timer_32ms(uint t)
    {
        publish(t);
    }
};

int main(int argc, char** argv)
{
    int pubCount = 3;
    if (argc == 2) pubCount = atoi(argv[1]);
    if (pubCount < 1 || pubCount > 100)
        return -1;

    printf("Starting publisher (%d runner)\n", pubCount);

    std::vector<HelloWorldPublisher*> publisher;

    for (int i = 0; i < pubCount; i++)
    {
        HelloWorldPublisher* app = new HelloWorldPublisher();
        if (!app->init())
            return -1;

        publisher.push_back(app);
    }

    uint t = 0;
    for(;;)
    {
        for (auto app : publisher)
        {
            app->on_timer_32ms(t);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(32));
        t++;
    }

    for (auto app : publisher)
    {
        delete app;
    }

    return 0;
}
