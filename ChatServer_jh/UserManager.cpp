
#include "User.h"
#include "UserManager.h"

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

void UserManager::Init()
{
}

bool UserManager::AddUser(const int sessionIndex, const char* id)
{
	User* user = new User(sessionIndex, id);
	user->SetState(User::STATE::LOGIN);
	mUserSessionMap.emplace(sessionIndex, user);
	mUserIDMap.emplace(id, user);
	return false;
}

bool UserManager::RemoveUser(const int sessionIndex, const char* id)
{
	User* user = FindUser(sessionIndex);

	if (user == nullptr)
		return false;

	mUserSessionMap.erase(sessionIndex);
	mUserIDMap.erase(user->GetID().c_str());

	return false;
}

User* UserManager::FindUser(const int sessionIndex)
{
	if (auto it = mUserSessionMap.find(sessionIndex); it != mUserSessionMap.end())
	{
		return it->second;
	}

	return nullptr;
}