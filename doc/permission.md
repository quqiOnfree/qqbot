# qqbot权限系统
## 组成部分
```cpp
//判断是否有默认群权限设置
bool hasGroupDefaultPermission(const std::string& permissionName);
//获取默认群权限设置
bool getGroupDefaultPermission(const std::string& permissionName);
//设置默认群权限设置
void setGroupDefaultPermission(const std::string& permissionName, bool boolean);

//判断是否有个性化群权限设置
bool hasSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
//获取个性化群权限
bool getSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
//设置个性化群权限
void setSingleGroupDefaultPermission(long long groupID, const std::string& permissionName, bool boolean);

//获取是否是operator
bool hasUserOperator(long long userID);
//设置operator
void setUserOperator(long long userID, bool boolean);
//获取operator列表
const qjson::JObject& getUserOperatorList();

```

## 详细api解释
### 默认群权限
#### bool hasGroupDefaultPermission(const std::string& permissionName);
- 获取默认群组中是否有 `权限名（permissionName）`

#### bool getGroupDefaultPermission(const std::string& permissionName);
- 获取默认群权限 `权限名（permissionName）` 是否对普通使用者开放

#### void setGroupDefaultPermission(const std::string& permissionName, bool boolean);
- 设置默认群权限 `权限名（permissionName）` 是否对普通使用者开放

### 个性化群权限（针对单个群）
#### bool hasSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
- 获取在 `群号（groupID）` 是否有个性化群权限 `权限名（permissionName）`
#### bool getSingleGroupDefaultPermission(long long groupID, const std::string& permissionName);
- 获取在 `群号（groupID）` 个性化群权限 `权限名（permissionName）` 是否对普通使用者开放
#### void setSingleGroupDefaultPermission(long long groupID, const std::string& permissionName, bool boolean);
- 设置在 `群号（groupID）` 个性化群权限 `权限名（permissionName）` 是否对普通使用者开放

### 设置Operator
#### bool hasUserOperator(long long userID);
- 获取 `用户QQ号（userID）` 是否是operator
#### void setUserOperator(long long userID, bool boolean);
- 设置 `用户QQ号（userID）` 是否是operator
#### const qjson::JObject& getUserOperatorList();
- 获取operator列表