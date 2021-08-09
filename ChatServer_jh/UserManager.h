#pragma once

#include "Header.h"

class User;
class UserManager
{
public:
	UserManager();
	~UserManager();


public:
	void Init();


public:
	bool AddUser(const int sessionIndex, const char* id);
	bool RemoveUser(const int sessionIndex, const char* id);

	User* FindUser(const int sessionIndex);


private:
	std::unordered_map<int, User*> mUserSessionMap;
	std::unordered_map<const char*, User*> mUserIDMap;
};

inline UserManager GUserManager;