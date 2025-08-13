# 使用字符串
本主题说明Windows如何支持`UI`元素、文件名等的`Unicode`字符串。
Windows使用`UTF-16`编码表示`Unicode`字符，其中每个字符都编码为一个或两个16位值。 为了将它们与8位`ANSI `字符区分开来，`UTF-16`字符被称为宽符。`WinNT.h`标头定义了以下 `typedef`。
```cpp
typedef wchar_t WCHAR;
```
若要声明宽字符文本或宽字符字符串字面量，请将`L`置于文本前面。
```cpp
wchar_t a = L'a';
wchar_t *str = L"hello";
```
下表列出了其他一些与字符串相关的类型定义：
|Typedef|定义|
|-|-|
|CHAR|`char`|
|PSTR或LPSTR|`char*`|
|PCSTR或LPCSTR|`const char*`|
|PWSTR或LPWSTR|`wchar_t*`|
|PCWSTR或LPCWSTR|`const wchar_t*`|

## `Unicode`和`ANSI`函数
Microsoft向Windows引入`Unicode`支持时，它通过提供两组并行API来缓解转换，一组用于`ANSI`字符串，另一组用于`Unicode`字符串。 例如，有两个函数可以设置窗口标题栏的文本：
* `SetWindowTestA`采用`ANSI`字符串
* `SetWindowTestW`采用`Unicode`字符串
在内部，`ANSI`版本将字符串转换为`Unicode`。Windows标头还定义了一个宏，该宏在定义预处理器符号`UNICODE`或`ANSI`版本时解析为`Unicode`版本。
```cpp
#ifdef UNICODE
#define SetWindowText  SetWindowTextW
#else
#define SetWindowText  SetWindowTextA
#endif
```
新应用程序应始终调用`Unicode`版本。许多世界语言都需要`Unicode`。如果使用`ANSI`字符串，则不可能对应用程序进行本地化。`ANSI`版本的效率也较低，因为操作系统必须在运行时将`ANSI`字符串转换为`Unicode`。根据偏好，可以显式调用`Unicode`函数，例如`SetWindowTextW`或使用宏。 最新的`Windows API`通常只有`Unicode`版本

## `TCHARs`
在某些情况下，根据目标平台，为`ANSI`或`Unicode`字符串编译相同的代码可能很有用。为此，Windows SDK提供将字符串映射到`Unicode`或`ANSI`的宏，具体取决于平台。
|宏|Unicode|ANSI|
|-|-|-|
|TCHAR|`wchar_t`|`char`|
|`TEXT("x")`或`_T("x")`|`L"x"`|`"x"`|
例如以下代码：
```cpp
SetWindowText(TEXT("My Application"));
```
解析为下列之一：
```cpp
SetWindowTextW(L"My Application"); // Unicode function with wide-character string.
SetWindowTextA("My Application");  // ANSI function.
```
`TEXT`和`TCHAR`宏目前不太有用，因为所有应用程序都应使用`Unicode`。

Microsoft C运行时库的标头定义一组类似的宏。例如，如果`_UNICODE`未定义，`_tcslen`解析为`strlen`；否则会解析为`wcslen`，这是`strlen`的宽字符版本。
```cpp
#ifdef _UNICODE
#define _tcslen     wcslen
#else
#define _tcslen     strlen
#endif
```
请注意：某些标头使用预处理器符号`UNICODE`，另一些标头使用带下划线前缀的`_UNICODE`。始终定义这两个符号。默认情况下，视觉对象`C++`在创建新项目时对其进行设置。
