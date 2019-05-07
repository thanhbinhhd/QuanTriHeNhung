# Quản trị dự án hệ nhúng theo chuẩn kỹ năng ITSS
![Arm](https://gloimg.gbtcdn.com/soa/gb/pdm-product-pic/Electronic/2018/12/20/goods_thumb-v16/20181220155903_34827.jpg)

# Đề tài
* Điều khiển cách tay robot bằng ps2 qua kết nối Bluetooth

## Group 01
* Hoang Minh Quang (minhquang4334)
* Le Cong Hau (HaJaU)
* Vu Hong Son (Parkboyoung)
* Le Thanh Binh


## Programming with Arduino IDE
* `https://www.arduino.cc/en/main/software`
## Controlling with Tera Term
* `./references/teraterm-4.102.exe`		
## Controlling
* `TO_LEFT` = `a` or `PSB_PAD_LEFT`;    // Spin to left
* `TO_RIGHT` = `d`  or `PSB_PAD_RIGHT`; // Spin to right
* `TO_UP` = `w` or `PSB_PAD_UP`;        // lift up
* `TO_DOWN` = `s` or `PSB_PAD_DOWN`;    // let down
* `TO_FRONT` = `8` or `PSB_TRIANGLE`;   // push to front
* `TO_BACK` = `2` or `PSB_CROSS`;       // pull to back
* `TO_CATCH` = `4` or `PSB_SQUARE`;     // Catch something
* `TO_RELEASE` = `6` or `PSB_CIRCLE`;   // Release
* `TO_RECORD` = `r` or `PSB_START`;     // Start/Reset recording
* `ENTER` = `13` or `PSB_SELECT`;       // Save current state
* `TO_REPLAY` = `e` or `PSB_R2`;        // Stop recording and start replaying 
* `TO_STOP_REPLAYING` = `x` or `PSB_L2`; // Strop replaying and change servo to first state
* `PAUSE` = `p` or `PSB_L1`;             // Pause while replaying
* `CONTINUE` = `c` or `PSB_R1`;          // Continue when paused
