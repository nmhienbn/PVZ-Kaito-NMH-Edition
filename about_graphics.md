- [PVZ 1 sprite sheets](#pvz-1-sprite-sheets)
  * [Giải thích ý nghĩa file](#gi-i-th-ch---ngh-a-file)
  * [Cách lấy](#c-ch-l-y)
- [Với PVZ 2 sprite sheet](#v-i-pvz-2-sprite-sheet)
  * [Giải thích ý nghĩa file](#gi-i-th-ch---ngh-a-file-1)
  * [Một số tool hữu ích](#m-t-s--tool-h-u--ch)
    + [SPC-Util](#spc-util)
    + [8A Gaming](#8a-gaming)
    + [TreToolKit](#tretoolkit)
- [Tạo GIF](#t-o-gif)
- [Tạo sprite sheets từ GIF](#t-o-sprite-sheets-t--gif)

Mãi mới biết mấy cái này =((. Làm hiệu ứng tốn 80% thời gian làm game.<br/>
Trong đó, tìm game asset trên mạng tốn 60% :v

Ở dưới đây --> có nghĩa là giải nén thành

# PVZ 1 PC sprite sheets
## Giải thích ý nghĩa file
Bản chất, khi tải 1 game pvz trên PC bản full (có thể dùng bản portable) sẽ có 1 file **.pak**<br/>
- **.pak** --> **.png**, **.jpg**, **.compiled**.
- **.compiled** --> **.fla**, **.jsfl**, **.xml**

Chi tiết các file:
- **.png**, **.jpg** là ảnh thì ai cũng biết rồi. **.png** thường có nền trong suốt vì là 1 bộ phận của cái gì đó.
- **.fla** sẽ ghép cái bộ phận đó lại (mở bằng Adobe Animate)
- **.xml** tương tự **.fla** nhưng hiệu ứng đơn giản hơn (thường chỉ gồm 1 ảnh) (nhưng chưa biết dùng ntn) hoặc các thông số game.
- **.jsfl** sẽ giúp compile ngược lại **.fla** và **.xml** trở lại **.compiled**

Về mặt lý thuyết chỉ cần decompile lại đến mức các file **.compiled** là được, không cần đến **.pak**
## Cách lấy
Công cụ giải nén và nén lại có nhiều trên mạng (bao gồm **PVZUtils**, [chi tiết](https://plantsvszombies.fandom.com/wiki/Modify_Plants_vs._Zombies))

Nói hơi dài nhưng chỉ cần **.fla** files là đủ. Trên [mạng](https://drive.google.com/file/d/1zztUOZWna5Hy2xI3tju1d7Hjtp59J4dI/view) cũng có sẵn luôn để tải, khỏi mất công giải nén.

Ở đây tôi chỉ dùng sprite sheet của zombie (vì chưa kịp chuyển hết sang pvz 2 cho đẹp)

Chi tiết lấy trong file *Zombie.fla*:
* 	credit1: 1-22
* 	credit2: 23-44
* 	walk1: 45-91
* 	walk2: 92-138
* 	walk3 (exactly dance): 455-504
* 	eat: 139-178
* 	die1: 179-217
* 	die2: 218-250
* 	die3: 293-429

# PVZ 1 Android

# Với PVZ 2 sprite sheet
## Giải thích ý nghĩa file
Giải nén:
- **.obb** / **.rsb** (Đều là dạng file binary để mã hóa) --> **.rsgp**, **.rton**
- **.rton**, **.dat** --> **.json**
- **resource.json** --> các **.json** (cho việc tách texture)
- **.rsgp** --> 
    - **.ptx** + **StructInfo.json** (nếu đuôi là _768, _1536; StructInfo.json là thông tin để giải nén)
    - **.PAM** (nếu đuôi là _common)
    - **.BNK** (nếu đuôi là _audio)
- **.ptx** + **StructInfo.json** --> **.png** (ở mức 1 texture)
- **.PAM** --> **.xfl**  + **LIBRARY** (để nhét các assets support cho .xfl) + **OtherInfo.json** (để xem đổi tên assets phù hợp)
- **.BNK** --> **.WEM** (một loại file audio)

Chú ý:
- **.xfl** + **LIBRARY** = **.fla**
- **.png** (ở mức 1 texture) + **.json** (tương ứng) --> **.PNG**s (các assets cho **LIBRARY**).
Ví dụ: *PlantCherryBomb_1536.png* + *PlantCherryBomb_1536.json*

## Một số tool hữu ích
Tham khảo rất nhiều tool tại [đây](https://ernestoam.fandom.com/wiki/Plants_vs._Zombies_2_Hacking_Tools).
### SPC-Util
[SPC-Util](https://www.youtube.com/watch?v=RvYLWEBOPG0)<br/>
Đồng tác giả với PVZ-Util
Chỉ cần kéo thả file cần giải nén (hoặc nén) vào file **SPCUtils.exe** (tên file có thể kèm tên tác giả nên có thể khác) và chọn option phù hợp (thường là 3 option cuối)

### 8A Gaming 
Các [tool](https://8auniverse.com/) của [8A Gaming - Youtube](https://www.youtube.com/@8AGamingOfficial/featured).<br/>
* MANIFESTGROUP tool: download resource.json để tách texture<br/>
* Texture Downloader Tool: download textures<br/>
* Simple Sprite Splitter Tool: tách texture<br/>
* 1BSR2PNGTool: .obb thành textures.<br/>

### TreToolKit
Tương tự SPCUtils, nhưng dễ dùng hơn.
Link: [Github](https://github.com/twinkles-twinstar/TwinStar.ToolKit), [Download](https://mega.nz/folder/vFNi2aKD#vpzZz9g0BAu7AcdO-CVEGg), [Youtube](https://www.youtube.com/watch?v=opBGagu4LOo&t=0s).
Chỉ cần kéo thả file cần giải nén (hoặc nén) vào file **SPCUtils.exe** (tên file có thể kèm tên tác giả nên có thể khác) và chọn option phù hợp

### Pop Studio
Thực chất .apk, .obb chỉ cần đổi đuôi sang .zip là giải nén đc.
Còn [PopStudio](https://github.com/YingFengTingYu/PopStudio_Old/releases/tag/v4.0) hỗ trợ giải nén hầu như
tất cả các định dạng file của PopCap Games. Chưa kể nó ở dạng UI thay vì cmd nên khá dễ sử dụng.

# Tạo GIF
**.fla** hoặc **.xfl + LIBRARY** có thể export được thành **.gif** với **Adobe Animate**(Chú ý Ctrl + J để chỉnh kích thước, và Export GIF nền và viền bao quanh trong suốt)

Chú ý với **.xfl + LIBRARY** của PVZ 2: cần copy các **.PNG** (assets) vào **LIBRARY** với tên phù hợp. Sau đó, copy tất cả layer từ Object **A_main** vào **main**. Nhớ ẩn các layer ở main. PVZ 2 tuy lằng nhằng hơn PVZ 1 nhưng **.xfl** có đánh dấu vị trí để biết animation từ frame nào đến frame nào.

# Tạo sprite sheets từ GIF
Sử dụng tool online [convert gif to spritesheet](https://onlinegiftools.com/convert-gif-to-sprite-sheet).
Các file **_blink** thì tạo bởi photoshop.