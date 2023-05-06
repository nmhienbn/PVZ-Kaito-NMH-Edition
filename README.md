# Plants Versus Zombie - Kaito-NMH-Edition
# Giới thiệu game
Plants versus Zombie là 1 game thuộc thể loại tower defense. Đắm mình vào cuộc chiến giữa cây cối và zombie, bạn hãy cố gắng hết sức để bảo vệ ngôi nhà của mình chỉ bằng những loại cây cối kì dị. Trồng cây và thu hoạch ngay kết quả là việc tiêu diệt các loại zombie đang bao vây ngôi nhà.

- [0. Cách tải game](#0-cách-tải-game)
  * [a. Cách 1: Không bao gồm code.](#a-cách-1-không-bao-gồm-code)
  * [b. Cách 2: Bao gồm code và có thể biên dịch.](#b-cách-2-bao-gồm-code-và-có-thể-biên-dịch)
- [1.    Bắt đầu game](#1bắt-đầu-game)
- [2.    Chọn level](#2chọn-level)
- [3.     Các thành phần trong game:](#3-các-thành-phần-trong-game)
- [4.     Cách chơi](#4-cách-chơi)
  * [a.   Các loại cây](#acác-loại-cây)
  * [b.   Các loại zombie](#bcác-loại-zombie)
- [5. Chiến thắng và thất bại](#5-chiến-thắng-và-thất-bại)
    + [Về đồ họa của game:](#về-đồ-họa-của-game)



# 0. Cách tải game
## a. Cách 1: Không bao gồm code.
Tải game (được nén thành .rar) tại link sau: https://drive.google.com/drive/u/0/folders/1awM0sfex1InCzlSA1ZIenKUGbhhC_hc0<br/>
Cách này tiết kiệm bộ nhớ và thời gian tải đi rất nhiều (khoảng 90MB).<br/>
Giải nén game vào một thư mục và bật pvz.exe lên và chơi.<br/>

## b. Cách 2: Bao gồm code và có thể biên dịch.
**Bước 1:** Clone repo này về (Khoảng 800 MB - do có cả lịch sử các phiên bản). <br/>
Hoặc Chọn Code -> Download Zip (Khoảng 90MB) 

![image](https://user-images.githubusercontent.com/65478890/236433509-2d4e5f97-ddc4-4daa-ae8f-9119c31b8d29.png)

**Tất cả code đều nằm trong thư mục src.<br/>
**Có thể nhảy đến bước 5 để chơi luôn.

**Bước 2:** Cài mingw. Ở đây tôi cài đặt phiên bản MinGW-W64 GCC-8.1.0 [x86_64-posix-seh](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z/download).<br/>
Một số chú ý: <br/>
- Nếu sử dụng trình biên dịch khác thì phải dịch lại icon: vào thư mục icon rồi dùng lệnh `windres main.rc -O coff main.res` trong cmd.<br/>
- Phải xóa ba dll của c++ trong thư mục chứa pvz.exe: `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`. Ba dll này chỉ dành cho người chơi không cài sẵn c++ trong máy.

**Bước 3:** Cài GNU make

**Bước 4:** Mở cmd trong thư mục chứa Makefile và gõ lệnh make.

**Bước 5:** Bật pvz.exe và chơi 

# 1.	Bắt đầu game
Click vào nút “CLICK TO START!” để bắt đầu chơi game.

![image](https://user-images.githubusercontent.com/65478890/235644003-664c5a3f-65e3-4f55-a308-1388f379226f.png)

 
Khi vào game lần đầu, người chơi hãy nhập tên mình để bắt đầu trò chơi. Sau đó click “OK” hoặc nhấn phím Enter để tiếp tục.

  ![image](https://user-images.githubusercontent.com/65478890/235644015-ef6de0e5-1f8a-46de-bd92-6b626c9327c2.png)



Và người chơi sẽ ngay lập tức nhận được 1 cây đầu tiên - Peashooter

 ![image](https://user-images.githubusercontent.com/65478890/235644035-f2874cd6-d245-4651-b140-aad2575600ac.png)


# 2.	Chọn level
Có tổng cộng 12 level với mức độ khó dễ khác nhau để bạn chơi. Khi di chuột lên 1 level, level đó sẽ phát sáng. Chỉ cần click chuột để chơi level đó.

 ![image](https://user-images.githubusercontent.com/65478890/235644049-1eb15dbb-4df4-44b3-a089-e1bee0b1ebee.png)


Tuy nhiên khi mới bắt đầu, bạn chỉ mở được 1 level. Chiến thắng lần lượt từng level để mở khóa và chơi level tiếp theo:

 ![image](https://user-images.githubusercontent.com/65478890/235644063-6498c78f-1215-4c73-90d5-12c78b098d4e.png)


Ngoài ra còn có 3 nút chức năng khác để người chơi đổi tên, đóng game hay thậm chí là reset lại tiến trình của game để trải nghiệm game lại từ đầu.

 ![image](https://user-images.githubusercontent.com/65478890/235644088-098a0d06-de52-4b9b-8e23-f455681d954b.png)



Khi vào game, có 2 loại level mà mọi người sẽ gặp: buổi sáng (bên trái) và buổi tối (bên phải)

 ![image](https://user-images.githubusercontent.com/65478890/235644138-5a1078e7-c457-4f9a-8c1f-14ff6f799d73.png)

●	Buổi sáng thì thi thoảng sẽ có sun (mặt trời) từ trên trời rơi xuống.
●	Còn buổi tối thì không có, vậy nên game sẽ khó hơn một chút.

Trước khi bắt đầu game, bạn sẽ được biết trước có những loại zombie gì sẽ xuất hiện.

 ![image](https://user-images.githubusercontent.com/65478890/235644159-c816334a-4125-434c-ad80-3dc7a9f1724a.png)

 
# 3.	 Các thành phần trong game:
-	Sun (Mặt trời) ![image](https://user-images.githubusercontent.com/65478890/235644190-003a31d5-e403-4f3e-9ecb-7edf40ad0eff.png)
: là đơn vị để mua plant (cây) trong game. Nhấp vào để thu. Và nên chú ý là nếu một sun tồn tại quá lâu, nó sẽ mờ dần rồi biến mất. Bạn sẽ không thể thu chúng sau khi chúng đã biến mất.
 
-	Sun bar (Thanh mặt trời) ![image](https://user-images.githubusercontent.com/65478890/235644218-cdeb1b32-201a-418c-b2aa-4d8269a5b63e.png)
: là bảng đếm số mặt trời mà người chơi đang có
 
-	Plant seeds: tất cả các giống cây là người chơi đang sở hữu. Để chọn một giống cây mang đi trồng, người chơi phải click vào plant seed hoặc nhấn phím 1, 2, 3, 4, 5 (như trong game đánh dấu) để chọn cây rồi trồng trên sân. Tất nhiên, bạn phải đủ sun để mua cây và plant seed phải có thời gian chờ khác nhau để trồng cây giống đó lần nữa.

 ![image](https://user-images.githubusercontent.com/65478890/235644263-1622c04b-d871-43f6-a489-9b4bff17780b.png)

-	Shovel (Xẻng): Là vật dụng vô cùng hữu hiệu để bạn có thể đào đi những cây không cần thiết và thay vào đó cây mới.

 ![image](https://user-images.githubusercontent.com/65478890/235644282-0532175a-5a42-4703-a621-44c645b39192.png)

-	Thanh tiến trình game (progress bar): Là thanh bạn có thể theo dõi để xem còn bao nhiêu zombie tiếp theo sẽ ra hay bạn đã chơi được bao nhiêu phần của level. Bên trái sẽ là level hiện tại bạn đang chơi.

 ![image](https://user-images.githubusercontent.com/65478890/235644299-10aba29c-3230-4051-81b9-f15dca3bab76.png)

-	Nút tua nhanh game (Turbo button): Nếu thấy game quá chậm hay quá dễ mà bạn cần tiết kiệm thời gian, hãy nhấn nút để trải nghiệm game với tốc độ x2 tốc độ gốc.
-	Và nếu thấy game quá đáng sợ ở tốc độ này, hãy nhấn lại để game trở về tốc độ bình thường.
●	Game ở tốc độ ban đầu:  ![image](https://user-images.githubusercontent.com/65478890/235644327-f6565bed-b5bb-430c-b257-7de0b6b697e0.png)

●	Game đang ở tốc độ nhanh gấp 2 ban đầu:  ![image](https://user-images.githubusercontent.com/65478890/235644339-df10c132-2432-4b48-840a-897e9fbab4b1.png)


-	Nút dừng game (Pause Button): Bạn có việc cần dừng game để lúc sau chơi tiếp ư? Hay bạn muốn điều chỉnh âm lượng của nhạc nền và hiệu ứng âm thanh? Quá đơn giản, game có thể dừng lại bằng cách click ![image](https://user-images.githubusercontent.com/65478890/235644416-9889bcca-ff91-4a8f-aa6d-b5fc2b4f1d41.png)  hoặc click đâu đó bên ngoài game.
-	Máy cắt cỏ (Mower): Nếu bạn để zombie lại quá gần nhà, máy cắt cỏ sẽ tự động kích hoạt và tiêu diệt tất cả zombie hiện tại đang ở hàng đó. Chú ý là mỗi hàng chỉ có một máy cắt cỏ thôi.

 ![image](https://user-images.githubusercontent.com/65478890/235644448-0aeeb06a-e177-4fd0-95c5-e0baee99b2d0.png)


-	Bố cục game cơ bản:

 ![image](https://user-images.githubusercontent.com/65478890/235644476-8fa38f42-60b6-4ea2-8fc0-c40a09353ed8.png)


 
# 4.	 Cách chơi
Trồng cây để tấn công và phòng thủ, không cho zombie bước vào nhà bạn.<br/>
Để trồng cây, Chọn hạt giống của cây đó (bàn phím hoặc click chuột) rồi đặt vào ô muốn trồng. <br/>
Có thể đào những cây đang sử dụng bằng xẻng.<br/>
Cây sẽ bị zombie ăn mất.

## a.	Các loại cây
Có 5 loại cây, lần lượt theo thứ tự có thể nhận được trong game là:

||Tên cây	|Chức năng|
|-|-|-|
|![image](https://user-images.githubusercontent.com/65478890/235644737-20b7af78-0629-41c1-aa9e-455a9377eaff.png)|Peashooter	|Mỗi lần bắn một đậu vào zombie.|
|![image](https://user-images.githubusercontent.com/65478890/235644753-b66d018a-3ffe-42fd-906c-28cf985c27b6.png)|Sunflower	|Tạo thêm mặt trời để trồng cây|
|![image](https://user-images.githubusercontent.com/65478890/235644793-a5039d3d-dfaa-4721-a5c7-4f64ddc63d7b.png)|Wall-nut	|Chịu được nhiều phát ăn của zombie để bảo vệ các cây khác.|
|![image](https://user-images.githubusercontent.com/65478890/235644820-78cdb1cc-a30e-4b2e-898d-99c3456bc634.png)|Snow Pea	|Mỗi lần bắn một đậu băng vào zombie. Đậu băng không tăng sát thương nhưng làm chậm tốc độ di chuyển và tốc độ ăn của zombie.|
|![image](https://user-images.githubusercontent.com/65478890/235644838-96dcb09f-a49a-4cc0-bfab-459eecd4b040.png)|Cherry Bomb	|Tiêu diệt tất cả zombie trong khu vực 3x3.|

 
## b.	Các loại zombie
-	Có 5 loại zombie:

||Tên zombie 	|Chức năng|
|-|-|-|
|![image](https://user-images.githubusercontent.com/65478890/235644897-e0f7fb94-3eee-4c20-8b13-751837dff84b.png)|Browncoat Zombie	|Ăn cây và não của bạn.|
|![image](https://user-images.githubusercontent.com/65478890/235644926-9ba95a6c-e4e9-4df3-b964-59e1dd2e0c81.png)|Flag Zombie	|Báo hiệu một lượng lớn zombie đang chuẩn bị ra.|
|![image](https://user-images.githubusercontent.com/65478890/235644956-95940313-e349-4ab2-a669-818030be4f5a.png)|Conehead Zombie	|Trâu gấp 3 lần zombie thường|
|![image](https://user-images.githubusercontent.com/65478890/235644975-45e2618b-84e7-4e5a-88f0-32228dbb2c76.png)|Buckethead Zombie	|Trâu gấp 6.5 lần zombie thường|
|![image](https://user-images.githubusercontent.com/65478890/235644995-cc8c1f86-335e-4272-a2ec-3af6b88dfca5.png)|Door Zombie	|Trâu gấp 6.5 lần zombie thường. Không bị ảnh hưởng bởi sự làm chậm của đạn băng nếu vẫn còn cửa.|
  
# 5. Chiến thắng và thất bại
- Bạn sẽ chiến thắng nếu tiêu diệt hết tất cả zombie trong ván đó.

![image](https://user-images.githubusercontent.com/65478890/235645245-c6c311d6-74b9-4fe2-b76a-930781573535.png)


- Bạn sẽ thất bại nếu có một zombie nào đó đi hết sân để vào nhà bạn.

![image](https://user-images.githubusercontent.com/65478890/235645414-1bbb03a1-4f25-42e0-b9cf-3267d4d11c27.png)

---
### Về đồ họa của game:
Các bộ phận của từng plants (mắt, lá, đầu, thân, ...), zombies (mắt, tóc, đầu, răng, thân, tay, chân, ...) và các thành phần khác được lấy tại [fandom](https://plantsvszombies.fandom.com/wiki/Main_Page)<br/>
Các bộ phận đó được ghép lại bằng Adobe Animate, và được chỉnh sửa bởi Adobe Photoshop --> Tạo thành file gif.<br/>
Để tạo thành sprite sheet, sử dụng trang https://onlinegiftools.com/convert-gif-to-sprite-sheet để chuyển đổi từ gif. <br/>
