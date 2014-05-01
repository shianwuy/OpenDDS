/*
 * $Id$
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/Service_Participant.h>

#include "DataReaderListener.h"
#include "MessengerTypeSupportC.h"
#include "MessengerTypeSupportImpl.h"

#include <iostream>

DataReaderListenerImpl::DataReaderListenerImpl()
  : num_samples_(0)
{
}

DataReaderListenerImpl::~DataReaderListenerImpl()
{
}

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
throw(CORBA::SystemException)
{
  try {
    Messenger::MessageDataReader_var message_dr =
      Messenger::MessageDataReader::_narrow(reader);

    if (CORBA::is_nil(message_dr.in())) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: on_data_available()")
                 ACE_TEXT(" ERROR: _narrow failed!\n")));
      ACE_OS::exit(-1);
    }

    Messenger::MessageSeq messages;
    DDS::SampleInfoSeq info;

    DDS::ReturnCode_t error = message_dr->take(messages,
                                               info,
                                               DDS::LENGTH_UNLIMITED,
                                               DDS::ANY_SAMPLE_STATE,
                                               DDS::ANY_VIEW_STATE,
                                               DDS::ANY_INSTANCE_STATE);

    if (error == DDS::RETCODE_OK) {

      for (unsigned int i = 0; i < messages.length(); ++i) {
        const DDS::SampleInfo& si = info[i];
        if (si.valid_data) {
          const Messenger::Message& message = messages[i];
          ++num_samples_;

          // output for console to consume
          std::cout << "Message: subject = " << message.subject.in()
                    << " subject_id = " << message.subject_id
                    << " count = " << message.count << '\n';
          // also track it in the log file
          ACE_DEBUG((LM_DEBUG,
                     ACE_TEXT("%N:%l: Message: subject = %C ")
                     ACE_TEXT("subject_id = %d ")
                     ACE_TEXT("count = %d\n"),
                     message.subject.in(),
                     message.subject_id,
                     message.count));

          for (CORBA::ULong i = 0; i < message.data.length(); ++i) {
            if ((message.subject_id == 1 &&
                 (message.data[i] != i % 256)) ||
                (message.subject_id == 2 &&
                 (message.data[i] != 255 - (i % 256)))) {
              std::cout << "ERROR: Bad data at index " << i << " subjid "
                        << message.subject_id << " count " << message.count
                        << "\n";
              break;
            }
          }
        } else if (si.instance_state == DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: instance is disposed\n")));

        } else if (si.instance_state == DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) {
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: instance is unregistered\n")));

        } else {
          ACE_ERROR((LM_ERROR,
                     ACE_TEXT("%N:%l: on_data_available()")
                     ACE_TEXT(" ERROR: unknown instance state: %d\n"),
                     si.instance_state));
        }
      }
    } else {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: on_data_available()")
                 ACE_TEXT(" ERROR: unexpected status: %d\n"),
                 error));
    }

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in on_data_available():");
    ACE_OS::exit(-1);
  }
}

void DataReaderListenerImpl::on_requested_deadline_missed(
  DDS::DataReader_ptr,
  const DDS::RequestedDeadlineMissedStatus &)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_requested_deadline_missed()\n")));
}

void DataReaderListenerImpl::on_requested_incompatible_qos(
  DDS::DataReader_ptr,
  const DDS::RequestedIncompatibleQosStatus &)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_requested_incompatible_qos()\n")));
}

void DataReaderListenerImpl::on_liveliness_changed(
  DDS::DataReader_ptr,
  const DDS::LivelinessChangedStatus& status)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_liveliness_changed() alive=%d, not alive=%d\n"),
             status.alive_count, status.not_alive_count));
}

void DataReaderListenerImpl::on_subscription_matched(
  DDS::DataReader_ptr,
  const DDS::SubscriptionMatchedStatus &)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_subscription_matched()\n")));
}

void DataReaderListenerImpl::on_sample_rejected(
  DDS::DataReader_ptr,
  const DDS::SampleRejectedStatus&)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_sample_rejected()\n")));
}

void DataReaderListenerImpl::on_sample_lost(
  DDS::DataReader_ptr,
  const DDS::SampleLostStatus&)
throw(CORBA::SystemException)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%N:%l: INFO: on_sample_lost()\n")));
}
