# Dự án Đồng Tâm

## Sơ đồ tổng quát

![OverviewPicture.](/Image/DongTamBlockDiagram_Overview.drawio.png)

## Yêu cầu

- Điều khiển đóng cắt 16 van khí
- Đọc tín hiệu kiểm tra van có bị hư hay không
- Sử dụng thời gian thực (chưa rõ mục đích)
- Gửi thông số áp suất lúc kích van (gửi giá trị đỉnh áp suất lên server)
- Mở rộng RS485 thêm các van


## Board chính

### Các thành phần

- MOSFET kích 16 valve khí [SI2300](https://icdayroi.com/si2300ds)(SL:16)
- Diode xả ngược bảo vệ MOSFET valve khí [1N4148WS](https://www.thegioiic.com/1n4148ws-diode-chinh-luu-0-15a-75v) (SL:16)
- IC mở rộng output [74HC595](https://www.thegioiic.com/74hc595d-cn) (SL:2)
- IC mở rộng input [74HC165](https://www.thegioiic.com/74hc165d-ic-shift-register-1-element-8-bit-complementary-16-soic)(SL:2)
- Đọc tín hiệu 16 van gửi về ( 14TSSOP)
[TL084IPT](https://www.thegioiic.com/tl084ipt-ic-opamp-j-fet-amplifier-4-circuit-4mhz-14-tssop) (số lượng 4).
- Đọc áp suất cảm biến (I2C) (AMS5915 khách hàng cung cấp)
- RS485(không cách ly vì thiếu không gian) [SP3485](https://www.thegioiic.com/sp3485en-line-transceiver-8-soic)(SL:1)
- RTC  [DS3231](https://www.thegioiic.com/ds3231sn-ic-rtc-clock-calendar-16-soic) và [PCF8563](https://www.thegioiic.com/pcf8563t-ic-rtc-clock-calendar-8-soic) (PCF8563 chồng footprint với DS3231)(SL:1)
- Biến trở số (SPI) [MCP41010](http://linhkienviet.vn/mcp41010-sop8-potentiometers-10k-bien-tro-so-10k)(SL:1)
- IC nhớ lưu thông số ESP32 (I2C)(SL:1)
- Nguồn đôi 24V và 5V (Khách hàng cung cấp)
- EEPROM lưu thông số board ESP32 [24LC256](https://www.thegioiic.com/24lc256-i-sn-ic-eeprom-256kbit-8-soic) (SL:1)
- Khối chuyển từ áp sang dòng 4-20mA
- Cổng IDC 14 pin nối tới board hiển thị (SL:1)


## Board hiển thị (đã thiết kế sẵn)

- Màn hình 20x4 mở rộng bằng PCF8574 
- Còi buzzer
- Led bar
- 4 nút nhấn 


## Bảng kết nối chân

Pin Number | Function      | Device Pin | Pin Function | Device Name
----------|---------------|------------|--------------|----------------
11(PA1)   | GPIO Output   | 2 & 3      | DERE         | SP3485
12(PA2)   | UART TX 2     | 1          | RO           | 
13(PA3)   | UART RX 2     | 4          | DI           | 
15(PA5)   | GPIO Output   | 12         | Clock        | 74HC595
17(PA7)   | GPIO Output   | 14(IC1)   | Data         | 
18(PB0)   | GPIO Output   | 11         | Store        | 
33(PA12)  | GPIO Output   | 13         | OE           | 
25(PB12)  | GPIO Output   | 1          | CS           | MCP41010
26(PB13)  | SPI SCK 2     | 2          | SCK          | 
28(PB15)  | SPI MOSI 2    | 3          | MOSI         | 
30(PA9)   | UART TX 1     | 34(RXD0)   | UART RX      | ESP32
31(PA10)  | UART RX 1     | 35(TXD0)   | UART TX      | 
32(PA11)  | GPIO Ouput    | 1          | Load         | 74HC165
33(PA12)  | GPIO Output   | 15         | OE           | 
39(PB3)   | GPIO Input    | 9 (IC2)    | Data         | 
40(PB4)   | GPIO Output   | 2          | Clock        | 
42(PB6)   | I2C SCL 1     | 6 (PCF)    | I2C SCL      | "PCF8574 || DS3231"
43(PB7)   | I2C SDA 1     | 5(PCF)     | I2C SDA      | 
42(PB6)   | I2C SCL 1     | 4          | I2C SCL      | AMS5915
43(PB7)   | I2C SDA 1     | 3          | I2C SDA      | 
