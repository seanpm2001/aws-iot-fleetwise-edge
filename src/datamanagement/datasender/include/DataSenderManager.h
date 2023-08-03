// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "DataSenderProtoWriter.h"
#include "ICollectionScheme.h"
#include "ICollectionSchemeList.h"
#include "ISender.h"
#include "PayloadManager.h"
namespace Aws
{
namespace IoTFleetWise
{
namespace DataSender
{

using namespace Aws::IoTFleetWise::OffboardConnectivityAwsIot;
using namespace Aws::IoTFleetWise::OffboardConnectivity;
using namespace Aws::IoTFleetWise::DataManagement;

/**
 * @brief Class that implements data sender logic: data preprocessing and upload
 *
 * This class is not multithreading safe to the caller needs to ensure that the different functions
 * are called only from one thread. This class will be instantiated and used from the Data Sender
 * Manager Worker thread
 */
class DataSenderManager
{

public:
    DataSenderManager( std::shared_ptr<ISender> mqttSender,
                       std::shared_ptr<PayloadManager> payloadManager,
                       CANInterfaceIDTranslator &canIDTranslator,
                       unsigned transmitThreshold );

    /**
     * @brief Process collection scheme parameters and prepare telemetry data and rich sensor data for the upload
     */
    void processCollectedData( const TriggeredCollectionSchemeDataPtr triggeredCollectionSchemeDataPtr );

    /**
     * @brief Retrieve all the persisted data and hand it over to the correct sender
     */
    void checkAndSendRetrievedData();

private:
    std::shared_ptr<ISender> mMQTTSender;
    std::shared_ptr<PayloadManager> mPayloadManager;
    DataSenderProtoWriter mProtoWriter;
    CollectionSchemeParams mCollectionSchemeParams;
    std::string mCollectionSchemeID;

    std::string mProtoOutput;
    std::string mCompressedProtoOutput;

    unsigned mTransmitThreshold{ 0 }; // max number of messages that can be sent to cloud at one time

    /**
     * @brief Set up collectionSchemeParams struct
     * @param triggeredCollectionSchemeDataPtr collected data
     */
    void setCollectionSchemeParameters( const TriggeredCollectionSchemeDataPtr &triggeredCollectionSchemeDataPtr );

    /**
     * @brief Put collected telemetry data into protobuf in chunks. Initiates serialization, compression, and
     * upload for each partition.
     * @param triggeredCollectionSchemeDataPtr collected data
     */
    void transformTelemetryDataToProto( const TriggeredCollectionSchemeDataPtr &triggeredCollectionSchemeDataPtr );

    /**
     * @brief Serializes, compresses, and uploads proto output.
     */
    void uploadProto();

    /**
     * @brief Serializes data
     * @param output Output string
     * @return True if serialization succeeds
     */
    bool serialize( std::string &output );

    /**
     * @brief Compresses data
     * @param input Input data string
     * @return True if compression succeeds
     */
    bool compress( std::string &input );

    /**
     * @brief Forwards data from buffer to the provided sender
     * @param data Data to send
     * @param size Buffer size
     * @param sender sender to use for the upload
     * @return Success if upload succeeds
     */
    ConnectivityError send( const std::uint8_t *data, size_t size, std::shared_ptr<ISender> sender );

    /**
     * @brief Upload file from persistency folder
     * @param filename File to send
     * @param size File size
     * @param collectionSchemeParams object containing collectionScheme related metadata for data persistency and
     * transmission
     * @return Success if upload succeeds
     */
    ConnectivityError uploadPersistedFile( const std::string &filename,
                                           size_t size,
                                           CollectionSchemeParams collectionSchemeParams );
};

} // namespace DataSender
} // namespace IoTFleetWise
} // namespace Aws