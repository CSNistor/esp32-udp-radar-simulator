#ifndef WIFI_HPP
#define WIFI_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"

namespace radar
{
    /**
     * @brief C++ wrapper for ESP-IDF Wi-Fi Station mode.
     *
     * This class is responsible for:
     * - initializing the Wi-Fi stack in STA mode
     * - registering event handlers
     * - connecting to an access point
     * - waiting until connection succeeds or fails
     *
     * Design goal:
     * keep app_main() clean and move Wi-Fi state/logic inside one object.
     */
    class WiFiStation
    {
    public:
        /**
         * @brief Construct a WiFiStation object.
         *
         * @param ssid Wi-Fi SSID to connect to.
         * @param password Wi-Fi password to use.
         * @param maxRetries Maximum number of reconnect attempts after disconnect.
         */
        WiFiStation(const char *ssid, const char *password, int maxRetries = 5);

        /**
         * @brief Destroy the WiFiStation object.
         *
         * No purpose atm.
         */
        ~WiFiStation();

        /**
         * @brief Initialize Wi-Fi in STA mode and wait for connection result.
         *
         * This method performs the full connection flow:
         * - create event group
         * - initialize esp_netif
         * - create default event loop
         * - create default STA interface
         * - initialize Wi-Fi driver
         * - register handlers
         * - apply Wi-Fi configuration
         * - start Wi-Fi
         * - wait until connected or failed
         *
         * @return true if the device connected and obtained an IP address
         * @return false if connection failed after all retries
         */
        bool connect();

        /**
         * @brief Check whether the station is currently marked as connected.
         *
         * This reflects the internal state updated by the event handler.
         *
         * @return true if connected and IP was obtained
         * @return false otherwise
         */
        bool isConnected() const;

    private:
        /**
         * @brief Event bit set when the station successfully gets an IP address.
         */
        static constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;

        /**
         * @brief Event bit set when the station fails after all retries.
         */
        static constexpr EventBits_t WIFI_FAIL_BIT = BIT1;

        /**
         * @brief Log tag used by ESP_LOGx macros for this class.
         */
        static constexpr const char *TAG = "WiFiStation";

        /**
         * @brief Static ESP-IDF event callback entry point.
         *
         * ESP-IDF expects a C-style callback function. Since member functions
         * have a hidden 'this' pointer, they cannot be directly passed as callbacks.
         *
         * This static function acts as a bridge:
         * - it receives ESP-IDF events
         * - it casts the generic user argument back to WiFiStation*
         * - it forwards the event to the instance method handleEvent()
         *
         * @param arg User-provided context pointer
         * @param eventBase Event family (e.g. WIFI_EVENT, IP_EVENT)
         * @param eventId Specific event ID inside that family
         * @param eventData Optional event-specific payload
         */
        static void eventHandlerThunk(void *arg,
                                      esp_event_base_t eventBase,
                                      int32_t eventId,
                                      void *eventData);

        /**
         * @brief Instance-level event handler.
         *
         * This method contains the actual event handling logic for:
         * - station start
         * - disconnect
         * - got IP
         *
         * @param eventBase Event family
         * @param eventId Specific event ID
         * @param eventData Optional event data
         */
        void handleEvent(esp_event_base_t eventBase,
                         int32_t eventId,
                         void *eventData);

        /**
         * @brief Create the FreeRTOS event group used for Wi-Fi synchronization.
         *
         * The event group is used to signal:
         * - successful connection
         * - final failure after retries
         */
        void createEventGroup();

        /**
         * @brief Initialize the TCP/IP network interface layer.
         *
         * This prepares the network stack infrastructure used by Wi-Fi.
         */
        void initNetif();

        /**
         * @brief Create the default ESP-IDF event loop.
         *
         * The event loop is required so the system can deliver Wi-Fi/IP events
         * to the registered handlers.
         */
        void createDefaultEventLoop();

        /**
         * @brief Create the default Wi-Fi station network interface.
         *
         * This creates the standard STA interface used when ESP32 acts as
         * a Wi-Fi client connecting to an access point.
         */
        void createDefaultStaInterface();

        /**
         * @brief Initialize the ESP-IDF Wi-Fi driver.
         *
         * This prepares internal Wi-Fi driver resources but does not yet connect.
         */
        void initDriver();

        /**
         * @brief Register event handlers for Wi-Fi and IP events.
         *
         * This method subscribes the class to:
         * - generic Wi-Fi events
         * - station got IP event
         */
        void registerEventHandlers();

        /**
         * @brief Apply station mode and user-provided Wi-Fi credentials.
         *
         * This method:
         * - sets Wi-Fi mode to STA
         * - fills wifi_config_t
         * - applies SSID and password
         */
        void configureStation();

        /**
         * @brief Start the Wi-Fi driver.
         *
         * After start, the driver will emit WIFI_EVENT_STA_START,
         * and the event handler can trigger esp_wifi_connect().
         */
        void start();

        /**
         * @brief Wait until connection succeeds or fails.
         *
         * This method blocks on the event group until:
         * - WIFI_CONNECTED_BIT is set
         * - or WIFI_FAIL_BIT is set
         *
         * @return true on success
         * @return false on failure
         */
        bool waitForConnection();

    private:
        /**
         * @brief Wi-Fi SSID provided by the user.
         *
         * Pointer lifetime must remain valid while the object uses it.
         */
        const char *ssid_;

        /**
         * @brief Wi-Fi password provided by the user.
         *
         * Pointer lifetime must remain valid while the object uses it.
         */
        const char *password_;

        /**
         * @brief Maximum number of reconnect attempts after disconnect.
         */
        int maxRetries_;

        /**
         * @brief Current reconnect attempt counter.
         */
        int retryCount_;

        /**
         * @brief Whether the station is currently considered connected.
         *
         * This becomes true after IP_EVENT_STA_GOT_IP.
         */
        bool connected_;

        /**
         * @brief FreeRTOS event group used to synchronize connection result.
         */
        EventGroupHandle_t wifiEventGroup_;

        /**
         * @brief Handler instance for Wi-Fi events.
         *
         * Stored so we can optionally unregister later.
         */
        esp_event_handler_instance_t wifiEventInstance_;

        /**
         * @brief Handler instance for IP events.
         *
         * Stored so we can optionally unregister later.
         */
        esp_event_handler_instance_t ipEventInstance_;
    };
} // namespace radar

#endif // WIFI_HPP