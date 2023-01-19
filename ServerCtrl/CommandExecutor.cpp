#include "CommandExecutor.h"
#include <QStringList>
#include <QIterator>
#include <iostream>
#include <iomanip>
#include <QDateTime>

#define DATETIME_FORMAT             "yyyy-MM-dd HH:mm:ss"
#define DEFAULT_TIME_OUT_SECONDS    15

#define DO_WAIT(__future) \
    if (__future.wait_for(std::chrono::seconds(DEFAULT_TIME_OUT_SECONDS)) == std::future_status::timeout) \
    {   \
        std::cout << "error: timeout" << std::endl; \
        return; \
    }

#define CHECK_RESULT_ERROR(__pResult) \
    if (__pResult->errCode) \
    {   \
        std::cout << "error[" <<  __pResult->errCode << "]: " << __pResult->errString << std::endl; \
        return; \
    }


#define GET_NEXT_ARG(__args, __dst) \
    if (!popNextArg(__args, __dst)) \
        break;

#define CHECK_EMPTY_ARGS(__args) \
    if (!__args.empty()) \
        break;

CommandExecutor::CommandExecutor()
{

}

void CommandExecutor::execute(const std::string &command, Client &client)
{
    std::list<std::string> args;
    size_t  l = 0;
    do
    {
        size_t r = command.find_first_of(' ', l);
        if (r == std::string::npos)
            break;

        std::string str = command.substr(l, r - l);
        l = r + 1;
        if (!str.empty() && str != " ")
            args.push_back(str);

    } while (true);
    if (l != command.length())
    {
        std::string str = command.substr(l, command.length() - l);
        if (!str.empty() && str != " ")
            args.push_back(str);
    }

    execute(args, client);
}

void CommandExecutor::execute(std::list<std::string>& args, Client &client)
{
    do
    {
        std::string arg;
        GET_NEXT_ARG(args, arg)

        if (arg == "user")
        {
            executeAroundUser(args, client);
        }
        else if (arg == "blacklist")
        {
            executeAroundBlackList(args, client);
        }
        else if (arg == "quit")
        {
            exit(0);
        }
        else
        {
            std::cout << "unknown commond " << arg << std::endl;
            break;
        }
        return;
    } while (false);


    std::cout << "Usage:" << std::endl;
    std::cout << "  <command> " << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  user              for user management" << std::endl;
    std::cout << "  blacklist         for blacklist management" << std::endl;
    std::cout << "  quit              quit" << std::endl;
    std::cout << std::endl;
}

void CommandExecutor::executeAroundUser(std::list<std::string>& args, Client &client)
{
    do
    {
        std::string arg;
        GET_NEXT_ARG(args, arg)

        if (arg == "add")
        {
            std::string account;
            GET_NEXT_ARG(args, account)

            std::string password;
            GET_NEXT_ARG(args, password)

            CHECK_EMPTY_ARGS(args)

            auto future = client.addAccount(account, password);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::AddAccountResult *result1 = dynamic_cast<Client::AddAccountResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            std::cout << "add user " << account << " successful, user id: " << result1->userId << std::endl;
        }
        else if (arg == "list")
        {
            std::string filter;
            popNextArg(args, filter);

            CHECK_EMPTY_ARGS(args)

            auto future = client.listAccount(filter);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::ListAccountResult *result1 = dynamic_cast<Client::ListAccountResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            if (result1->accounts.empty())
            {
                std::cout << "0 records" << std::endl;
            }
            else
            {
                std::cout << std::setw(24) << "user id" << std::setw(24) << "account" << std::setw(16) << "disabled" << std::endl;
                for (auto& account : result1->accounts)
                {
                    std::cout << std::setw(24) << account.userId << std::setw(24) << account.account << std::setw(8) << account.disabled << std::endl;
                }
            }
        }
        else if (arg == "enable")
        {
            std::string account;
            GET_NEXT_ARG(args, account)

            CHECK_EMPTY_ARGS(args)

            auto future = client.enableAccount(account);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::EnableAccountResult *result1 = dynamic_cast<Client::EnableAccountResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            std::cout << "enable user " << account << " successful"  << std::endl;
        }
        else if (arg == "disable")
        {
            std::string account;
            GET_NEXT_ARG(args, account)

            CHECK_EMPTY_ARGS(args)

            auto future = client.disableAccount(account);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::DisableAccountResult *result1 = dynamic_cast<Client::DisableAccountResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            std::cout << "disable user " << account << " successful"  << std::endl;
        }
        else if (arg == "del")
        {
            std::string account;
            GET_NEXT_ARG(args, account)

            CHECK_EMPTY_ARGS(args)

            auto future = client.deleteAccount(account);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::DeleteAccountResult *result1 = dynamic_cast<Client::DeleteAccountResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            std::cout << "delete user " << account << " successful"  << std::endl;
        }
        else if (arg == "chpwd")
        {
            std::string account;
            GET_NEXT_ARG(args, account)

            std::string password;
            GET_NEXT_ARG(args, password)

            CHECK_EMPTY_ARGS(args)

            auto future = client.changeAccountPassword(account, password);
            DO_WAIT(future)

            std::unique_ptr<Client::IResult> result = future.get();
            Client::ChangeAccountPasswordResult *result1 = dynamic_cast<Client::ChangeAccountPasswordResult*>(result.get());
            assert(result1);
            CHECK_RESULT_ERROR(result1)
            std::cout << "the password of user  " << account << " has been changed" << std::endl;
        }
        else
        {
            std::cout << "unknown commond " << arg << std::endl;
            break;
        }
        return;
    } while (false);


    std::cout << "Usage:" << std::endl;
    std::cout << "  user <command> " << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  add       <user name>     <password>      add user" << std::endl;
    std::cout << "  list      [user name]                     list users" << std::endl;
    std::cout << "  chpwd     <user name>     <password>      change user password" << std::endl;
    std::cout << "  enable    <user name>                     enable user" << std::endl;
    std::cout << "  disable   <user name>                     disable user" << std::endl;
    std::cout << "  del       <user name>                     delete user" << std::endl;
    std::cout << std::endl;
}

void CommandExecutor::executeAroundBlackList(std::list<std::string> &args, Client &client)
{
    do
    {
        std::string arg;
        GET_NEXT_ARG(args, arg)

        if (arg == "ip")
        {
            std::string op;
            GET_NEXT_ARG(args, op)

            if (op == "list")
            {
                CHECK_EMPTY_ARGS(args)
                auto future = client.listBlockedIPAddresses();
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::ListBlockedIPAddressesResult *result1 = dynamic_cast<Client::ListBlockedIPAddressesResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                if (result1->ipAddresses.empty())
                {
                    std::cout << "0 records" << std::endl;
                }
                else
                {
                    std::cout << std::setw(24) << "IP address" << std::setw(24) << "Until" << std::endl;
                    for (auto& it : result1->ipAddresses)
                    {
                        std::cout << std::setw(24) << it.ip
                                  << std::setw(24) << QDateTime::fromSecsSinceEpoch(it.until).toString(DATETIME_FORMAT).toStdString()
                                  << std::endl;
                    }
                }
            }
            else if (op == "add")
            {
                std::string ip;
                GET_NEXT_ARG(args, ip)

                std::string duration;
                GET_NEXT_ARG(args, duration)

                CHECK_EMPTY_ARGS(args)

                unsigned n;
                try
                {
                    n = std::stoul(duration);
                } catch(...)
                {
                    std::cout << "invalid duration seconds";
                    break;
                }

                auto future = client.addBlockedIPAddress(ip, n);
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::AddBlockedIPAddressResult *result1 = dynamic_cast<Client::AddBlockedIPAddressResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                std::cout << "IP address " << ip << " is added to blacklist" << std::endl;
            }
            else if (op == "remove")
            {
                std::string ip;
                GET_NEXT_ARG(args, ip)

                CHECK_EMPTY_ARGS(args)

                auto future = client.removeBlockedIPAddress(ip);
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::RemoveBlockedIPAddressResult *result1 = dynamic_cast<Client::RemoveBlockedIPAddressResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                std::cout << "IP address " << ip << " is removed from blacklist" << std::endl;
            }
            else
            {
                std::cout << "unknown commond " << op << std::endl;
            }
        }
        else if (arg == "user")
        {
            std::string op;
            GET_NEXT_ARG(args, op)

            if (op == "list")
            {
                CHECK_EMPTY_ARGS(args)
                auto future = client.listBlockedAccountss();
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::ListBlockedAccountsResult *result1 = dynamic_cast<Client::ListBlockedAccountsResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                if (result1->accounts.empty())
                {
                    std::cout << "0 records" << std::endl;
                }
                else
                {
                    std::cout << std::setw(24) << "User Id" << std::setw(24) << "Account"  << std::setw(24) << "Until" << std::endl;
                    for (auto& it : result1->accounts)
                    {
                        std::cout << std::setw(24) << it.userId << std::setw(24) << it.account
                                  << std::setw(24) << QDateTime::fromSecsSinceEpoch(it.until).toString(DATETIME_FORMAT).toStdString()
                                  << std::endl;
                    }
                }
            }
            else if (op == "add")
            {
                std::string sid;
                GET_NEXT_ARG(args, sid)

                std::string duration;
                GET_NEXT_ARG(args, duration)

                CHECK_EMPTY_ARGS(args)
                User::ID id;
                try
                {
                    id = std::stoll(sid);
                } catch(...)
                {
                    std::cout << "invalid user id";
                    break;
                }

                unsigned n;
                try
                {
                    n = std::stoul(duration);
                } catch(...)
                {
                    std::cout << "invalid duration seconds";
                    break;
                }


                auto future = client.addBlockedAccount(id, n);
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::AddBlockedAccountResult *result1 = dynamic_cast<Client::AddBlockedAccountResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                std::cout << "User " << id << " is added to blacklist" << std::endl;
            }
            else if (op == "remove")
            {
                std::string sid;
                GET_NEXT_ARG(args, sid)

                CHECK_EMPTY_ARGS(args)
                User::ID id;
                try
                {
                    id = std::stoll(sid);
                } catch(...)
                {
                    std::cout << "invalid user id";
                    break;
                }

                auto future = client.removeBlockedAccount(id);
                DO_WAIT(future)

                std::unique_ptr<Client::IResult> result = future.get();
                Client::RemoveBlockedAccountResult *result1 = dynamic_cast<Client::RemoveBlockedAccountResult*>(result.get());
                assert(result1);
                CHECK_RESULT_ERROR(result1)
                std::cout << "User " << id << " is removed from blacklist" << std::endl;
            }
            else
            {
                std::cout << "unknown commond " << op << std::endl;
            }
        }
        else
        {
            std::cout << "unknown commond " << arg << std::endl;
            break;
        }
        return;
    } while (false);


    std::cout << "Usage:" << std::endl;
    std::cout << "  blacklist <ip | user> <command> " << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands about ip:" << std::endl;
    std::cout << "  list                                            list all blocked ip addresses" << std::endl;
    std::cout << "  add        <ip address>  <duration seconds>     add to blacklist" << std::endl;
    std::cout << "  remove     <ip address>                         remove ip address from blacklist" << std::endl;

    std::cout << "  blacklist account <command> " << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands about user:" << std::endl;
    std::cout << "  list                                        list all blocked accounts" << std::endl;
    std::cout << "  add        <user id>  <duration seconds>    add to blacklist" << std::endl;
    std::cout << "  remove     <user id>                        remove account from blacklist" << std::endl;
    std::cout << std::endl;
}

bool CommandExecutor::popNextArg(std::list<std::string> &args, std::string &dst)
{
    if (args.empty())
        return false;
    dst = args.front();
    args.pop_front();
    return true;
}
