# Tựa đề legit
## FileSystem::FileSystemAlt
Quản lý bảng mô tả file.

<br>

### Thiết kế
Sử dụng một mảng các ```Node``` để lưu trữ thông tin về các file đang mở. Mỗi ```Node``` chứa tên file, mô tả file, loại file.

Prototype: ```fileSystemAlt.h```
```c
class Node
{
    char* name; // Tên file
    OpenFile* file; // Mô tả file
    int type;   // Loại file
}

class FileSysAlt
{
private:
    Node **globalFileTable;
    int count;  // Số lượng file đang mở
public:
    int fOpen(char* name);
    int fClose(int id);
    int fWrite(int id, char* buffer, int size);
    int fRead(int id, char* buffer, int size);
    int fSeek(int id, int pos, int type);
    int fDelete(char* name);
    // TBD
}
```
<br>

### Xử lý từ SystemCall
Sau khi qua xử lý các kiểu(đọc tiếp), các biến truyền vào syscall (dạng `SC_`) trong các register 4++ được truyền tiếp vào các hàm trong class `FileSystem`, kế thừa `FileSystemAlt` & `OpenFile`, cụ thể:  

<br>

`int fOpen(char *name, int mode)`: Mở một file handle. File được mở bằng UNIX syscall `open()` thông qua hàm mở file trong `sysdep.cc`.\
Mở file không test đã mở hay gì hết, chỉ mở.

* `type` có thể tương ứng với:
0. `RO`: chỉ đọc.
1. `RW`: đọc và ghi.

* Trả về 0 nếu mở thành công, -1 nếu không có file hoặc không thể mở (chỉ có không có file)

Note bé tẹo:+O_RDONLY trong `sysdep.cc`

<br>

`int fClose(OpenFileId id)`: Xoá entry thứ `id` & gọi destructor `~Node() -> ~OpenFile()`, trả về 0 nếu có trong bảng mô tả, trả về -1 nếu entry = NULL.

<br>

`int fRead(char* buff, int count, OpenFileId id)`:\
Đọc `count` byte từ `id` vào `buff` qua phương thức `OpenFile::Read(char*, int)` cho file và `SynchConsole::Read(char*, int)` cho ConsoleInput.\
Trả về số byte thực đọc, `-2` nếu `count` > số thực

<br>

`int fWrite(char* buff, int count, OpenFileId id)`: Tương tự `fRead()`, thay vì đọc thì ghi

<br>

`int fSeek(int pos, OpenFileId)`: Di chuyển fp tới `pos`. (không thực sự di chuyển fp bằng lseek() mà chỉ thay\
đổi `int OpenFile::currentOffset`)

<br>

`int fDelete(char* name)`: Xoá file nếu không có tên trong bảng

<br>

### Xử lý các kiểu trong `exception.cc`

<br>

`User2System() & System2User()` được sửa lại để đọc `'\0'` vì yêu cầu đề file nhị phân. Console vẫn có thể đọc ASCII như bình thường nhưng `cat()` sẽ in hết file, giống cat (UNIX).\
`test.bin` là file mẫu case này, mở hex lên xem.

<br>

Xử lý trả về `-2` của `Read()` và `Write()` được thực hiện ở chính syscall handler (`exception.cc`) bằng ma thuật.\
User phải tự handle số byte cần đọc và ghi, vd chạy được là dùng `Seek()` như trong trong `copy.c`.\
Đọc quá NachOS không chết user chết (nghĩ được quả case nào hay hay test xem).

<br>

Và liên quan đến chuỗi nếu không quên thì đều có case ?= null.\
hỏi nếu không hiểu bất kỳ cái gì pls, test nếu vui tính hehe. được thì chiều t5 ngồi làm report?