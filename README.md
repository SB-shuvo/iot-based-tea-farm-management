# IoT-based Tea Farm Management Using ESP32, RFID Cards, Load Cell, and OLED Display

### Components Used:
- ESP32 WROOM 32 development board
- Load cell
- Hx711 amplifier module
- RFID Card and Reader
- 128×64 OLED display
- Bidirectional Voltage Level Converter (5V <-> 3.3V)
- Male Header
- 7805 Regulator
- RGB LED
- Push buttons

## Functions
- **Load Cell:** The load cell is used to measure the weight of the tea leaves as they are collected.
- **ESP32:** It controls all the components, connects to the local Wi-Fi network, and allows the system to communicate with external services like Google Sheets.
- **RFID Scanner:** The RFID scanner is used to identify the workers. Each worker has an RFID card that they can scan. The scanner reads the RFID tag's unique identifier.
- **OLED Display:** A display screen can show relevant information such as the worker's ID and the weight of the tea leaves, they've collected. This gives instant feedback to the worker and helps them ensure accuracy.
- **Bi-directional Level Converter:** In this system components works in different voltage level. The HX711 amplifier of the load cell operates at 5 V while the ESP32, OLED display, and RFID scanner works at 3.3 V. So, we used a level converter to convert between these two voltage levels.
- **Data Upload to Google Sheets:** The collected data, including the worker's ID and the corresponding weight, is sent to a Google Sheets spreadsheet. This allows for centralized data storage and easy analysis. 


## Description of Uploaded files
- circuit diagram.pdf has the schematic diagram of the whole system
- Tea_farm_management_esp32_code has the code that was uploaded to ESP32
- write_rfid_esp32 has the code used for writing ID numbers to RFID cards
- GPIO_Nums_ESP32-WROOM-DA-DEVELOPMENT-BOARD.jpg can be used to find the GPIO number corresponding to a pin number (for example: TXD = GPIO1)
- dataAnalysisGoogleSheet.xlsx is the downloaded google sheet where the sample data were uploaded
- upload_to_google_sheet_script.txt is the Google App Script used along with Google Sheet for uploading the data from ESP32 to Google Sheet. The instructions for setting up the script can be found here: [https://youtu.be/KPOFncRDiHQ?si=cw44Ts_JBDaSdF4q]
- altium_pcb_files.zip has all the Altium Designer project files for the PCB implementation of the project. The PCB layout has  Please note that the pin sequence of the OLED display needs to be reversed. And the HX711

## Acknowledgements
- Himanshu Sharma [https://github.com/himanshus2847/IoT-Attendance-System-using-RFID]
- Rui Santos [https://randomnerdtutorials.com/esp32-load-cell-hx711/]

## Demonstration
[https://youtu.be/nc2dN1RnbW0?si=kH0zhom45LqhLJK2]
