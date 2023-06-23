# qqbot命令系统
## 组成部分
```cpp
//群消息处理函数模板
//args: groupID, senderID, commandName, Args
using GroupHandler = std::function<void(long long, long long, const std::string&, std::vector<std::string>)>;
		
//单人消息处理函数模板
//args: senderID, commandName, Args
using UserHandler = std::function<void(long long, const std::string&, std::vector<std::string>)>;


//添加群命令
void addCommand(const std::string& commandName,
    GroupHandler handler,
    const std::string& commandFormat,
    const std::string& description);

//添加私聊命令
void addCommand(const std::string& commandName,
    UserHandler handler,
    const std::string& commandFormat,
    const std::string& description);

//群消息处理
void groupExcute(long long groupID,
    long long senderID,
    const std::string& command,
    std::vector<std::string> args);

//私聊消息处理
void userExcute(long long senderID,
    const std::string& command,
    std::vector<std::string> args);

//命令分割
static void splitCommand(const std::string& command, std::string& commandHead, std::vector<std::string>& args);

```
