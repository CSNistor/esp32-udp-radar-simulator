#ifndef UDP_SENDER_HPP
#define UDP_SENDER_HPP

#include <cstdint>
#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "lwip/sockets.h"

namespace radar
{
    /**
     * @brief Simple UDP sender for ESP32 using BSD sockets (lwIP).
     *
     * This class is responsible for:
     * - creating a UDP socket
     * - configuring destination IP and port
     * - sending messages over UDP
     *
     * Design goal:
     * keep networking logic encapsulated and reusable.
     */
    class UdpSender
    {
    public:
        /**
         * @brief Construct a new Udp Sender object.
         *
         * @param destIp Destination IPv4 address (string format, e.g. "192.168.1.100")
         * @param destPort Destination UDP port
         */
        UdpSender(const char *destIp, uint16_t destPort);

        /**
         * @brief Destroy the Udp Sender object.
         *
         * Closes the socket if it was opened.
         */
        ~UdpSender();

        /**
         * @brief Initialize the UDP socket and destination address.
         *
         * Must be called before sending data.
         *
         * @return true if initialization succeeded
         * @return false if socket creation or address setup failed
         */
        bool init();

        /**
         * @brief Send a single UDP message.
         *
         * @param data Pointer to data buffer
         * @param length Size of the data buffer
         * @return true if send was successful
         * @return false otherwise
         */
        bool send(const char *data, size_t length);

        /**
         * @brief Start the UDP sender as a FreeRTOS task.
         *
         * @param taskName Name visible in the RTOS/task list
         * @param stackSize Stack size in words/bytes depending on ESP-IDF API expectation
         * @param priority FreeRTOS task priority
         * @return true if task creation succeeded
         * @return false otherwise
         */
        bool startTask(const char *taskName = "UdpSenderTask",
                       uint32_t stackSize = 4096,
                       UBaseType_t priority = 5);

    private:
        /**
         * @brief Static task entry function used by FreeRTOS.
         *
         * FreeRTOS requires a C-style function pointer.
         * This method converts the generic argument back to UdpSender*
         * and forwards execution to the instance method run().
         */
        static void taskEntry(void *arg);

        /**
         * @brief Main task loop for periodic UDP sending.
         */
        void run();

        /**
         * @brief Create UDP socket.
         *
         * @return true if socket created successfully
         * @return false otherwise
         */
        bool createSocket();

        /**
         * @brief Configure destination address (sockaddr_in).
         *
         * Converts string IP to binary and sets port.
         *
         * @return true if configuration succeeded
         * @return false otherwise
         */
        bool configureDestination();

    private:
        /**
         * @brief Destination IP address (string format).
         */
        const char *destIp_;

        /**
         * @brief Destination port.
         */
        uint16_t destPort_;

        /**
         * @brief Socket file descriptor.
         */
        int sock_;

        /**
         * @brief Destination socket address structure.
         */
        struct sockaddr_in destAddr_;

        /**
         * @brief Simple message counter (for testing).
         */
        int counter_;
    };

} // namespace radar

#endif // UDP_SENDER_HPP