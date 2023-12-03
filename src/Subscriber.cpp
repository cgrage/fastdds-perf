#include "idl/MessagePubSubTypes.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

class HelloWorldSubscriber
{
private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    DataReader* reader_;
    Topic* topic_;
    TypeSupport type_;

    class SubListener : public DataReaderListener
    {
    public:
        Message hello_;
        std::atomic_uint samples_;

        SubListener() : samples_(0) { }
        ~SubListener() override { }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            if (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                    samples_++;
            }
        }

    } listener_;

public:

    HelloWorldSubscriber()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new MessagePubSubType())
    {
    }

    virtual ~HelloWorldSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the subscriber
    bool init()
    {
        DomainParticipantQos dpQos;
        dpQos.name("Participant_subscriber");

        bool useShmTransport = false;
        if (useShmTransport)
        {
            std::shared_ptr<SharedMemTransportDescriptor> shm_transport = std::make_shared<SharedMemTransportDescriptor>();

            // [OPTIONAL] ThreadSettings configuration
            // shm_transport->default_reception_threads(eprosima::fastdds::rtps::ThreadSettings{-1, 0, 0, -1});
            // shm_transport->set_thread_config_for_port(12345, eprosima::fastdds::rtps::ThreadSettings{-1, 0, 0, -1});
            // shm_transport->dump_thread(eprosima::fastdds::rtps::ThreadSettings{-1, 0, 0, -1});

            dpQos.transport().user_transports.push_back(shm_transport);
            dpQos.transport().use_builtin_transports = false;
        }

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, dpQos);
        if (participant_ == nullptr)
            return false;

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("MessageTopic", "Message", TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
            return false;

        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr)
            return false;

        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if (reader_ == nullptr)
            return false;

        return true;
    }

    uint32_t statSamples_ = 0;

    //!Run the Subscriber
    void on_timer_1s()
    {
        uint32_t samples = listener_.samples_;
        uint32_t diff = samples - statSamples_;
        statSamples_ = samples;

        printf("%0.f messages per second\n", diff / 1.0f);
    }
};

int main(int argc, char** argv)
{
    int subCount = 1;
    if (argc == 2) subCount = atoi(argv[1]);
    if (subCount < 1 || subCount > 100)
        return -1;

    printf("Starting subscriber (%d runner)\n", subCount);

    std::vector<HelloWorldSubscriber*> subscriber;

    for (int i = 0; i < subCount; i++)
    {
        HelloWorldSubscriber* app = new HelloWorldSubscriber();
        if (!app->init())
            return -1;

        subscriber.push_back(app);
    }

    for(;;)
    {
        for (auto app : subscriber)
        {
            app->on_timer_1s();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    for (auto app : subscriber)
    {
        delete app;
    }

    return 0;
}
