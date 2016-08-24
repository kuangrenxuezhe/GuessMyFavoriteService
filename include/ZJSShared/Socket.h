#ifndef _ZJS_SOCKET_H_
#define _ZJS_SOCKET_H_
#include "Sys.h"

namespace nsZJSSocket 
{
    class Socket
    {
        Socket(int connectRetries=3, int connectTimeout=5, int sendTimeout=5, int recvTimeout=10)
        {
#ifdef WIN32
            WSADATA wsa;
            if(WSAStartup(MAKEWORD(2, 1), &wsa) != 0)
            {
                return;
            }
#endif
            m_sockfd = -1;
            m_closed = false;
            m_option = 0;
            m_ipaddr = 0;
            m_port = 0;

            m_connectRetries = connectRetries;
            m_connectTimeout = connectTimeout;
            m_sendTimeout = sendTimeout;
            m_recvTimeout = recvTimeout;
            m_errcode = 0;
            m_param = 0;
        }

        ~Socket()
        {
#ifdef WIN32
            WSACleanup();
#endif
            Close();
        }

        int32_t Open()
        {
            m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(m_sockfd == -1) { return false; }

            return true;
        }

        void Close()
        {
            if(m_sockfd != -1)
            {
#ifdef WIN32
                closesocket(m_sockfd);
#else
                close(m_sockfd);
#endif
                m_sockfd = -1;
            }
        }
        void Option(m_sendTimeout, m_recvTimeout)
        {
            uint32_t ioctlopt = 1;
            long bufsize = 65536;
            long timeout = 0;
#ifdef WIN32
            // ioctlsocket(m_sockfd, FIONBIO, &ioctlopt);
#else
            fcntl(m_sockfd, F_SETFL, O_NONBLOCK);
#endif
            setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(bufsize));
            setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));

            timeout = m_sendTimeout;
            setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

            timeout = m_recvTimeout;
            setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        }

        int32_t Connect()
        {
            struct sockaddr_in sa;
            long retries, rv;
#ifdef WIN32
            struct timeval exptime;
            fd_set wset;
#else
            struct pollfd fds;
            long exptime;
#endif

            if(m_ipaddr == -1)
            {
                return false;
            }

            memset(&sa, 0, sizeof(sa));
            memcpy(&sa.sin_addr, &m_ipaddr, 4);
            sa.sin_family = AF_INET;
            sa.sin_m_port = htons(m_port);

            retries = 0;
            if(connect(m_sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
            {
                while(retries < m_connectRetries)
                {
#ifdef WIN32
                    FD_ZERO(&wset);
                    FD_SET(m_sockfd, &wset);

                    exptime.tv_sec = m_connectTimeout/m_connectRetries;
                    exptime.tv_usec = 0;
                    rv = select(m_sockfd+1, 0, &wset, 0, &exptime);
#else
                    fds.fd = m_sockfd;
                    fds.events = POLLWRNORM;

                    exptime = m_connectTimeout*1000/m_connectRetries;
                    rv = poll(&fds, 1, exptime);
#endif
                    if(rv < 0)
                    {
                        return false;
                    }
                    else if(rv == 0)
                    {
                        retries ++;
                    }
                    else
                    {
                        return true;
                    }
                }

                return false;
            }

            return true;
        }

        int32_t IsConnect()
        {
#ifdef WIN32
            struct timeval exptime;
            fd_set wset;
#else
            struct pollfd fds;
            long exptime;
#endif
            int32_t rv;

            if(m_ipaddr == -1)
            {
                return false;
            }
#ifdef WIN32
            FD_ZERO(&wset);
            FD_SET(m_sockfd, &wset);

            exptime.tv_sec = m_connectTimeout/m_connectRetries;
            exptime.tv_usec = 0;
            rv = select(m_sockfd+1, 0, &wset, 0, &exptime);
#else
            fds.fd = m_sockfd;
            fds.events = POLLWRNORM;

            exptime = m_connectTimeout*1000/m_connectRetries;
            rv = poll(&fds, 1, exptime);
#endif
            if(rv < 0)
            {
                m_errcode = errno;
                return false;
            }

            return true;
        }

        int32_t SendMessage(int8_t* request, uint32_t length)
        {
            long timer;
#ifdef WIN32
            struct timeval exptime;
            fd_set wset;
#else
            struct pollfd fds;
            long exptime;
#endif
            int32_t rv;

            timer = 0;

            while(length > 0)
            {
#ifdef WIN32
                FD_ZERO(&wset);
                FD_SET(m_sockfd, &wset);

                exptime.tv_sec = 1;
                exptime.tv_usec = 0;
                rv = select(m_sockfd+1, 0, &wset, 0, &exptime);
#else
                fds.fd = m_sockfd;
                fds.events = POLLWRNORM;

                exptime = 1000;
                rv = poll(&fds, 1, exptime);
#endif
                if(rv < 0)
                {
                    m_errcode = errno;
                    return SYS_SOCK_ERRIO;
                }
                else if(rv == 0)
                {
                    if(++timer >= (long)m_sendTimeout)
                    {
                        return SYS_SOCK_ERRTIMEO;
                    }
                    continue;
                }

#ifdef WIN32
                rv = send(m_sockfd, request, length, 0);
#else
                do
                {
                    rv = write(m_sockfd, request, length);
                }
                while(rv < 0 && errno == EINTR);
#endif
                if(rv < 0)
                {
#ifndef WIN32
                    if(errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        if(timer >= (long)m_sendTimeout)
                        {
                            return SYS_SOCK_ERRTIMEO;
                        }
                        continue;
                    }
#endif
                    m_errcode = errno;
                    return SYS_SOCK_ERRIO;
                }
                else if(rv == 0)
                {
                    return SYS_SOCK_ERRFIN;
                }
                else
                {
                    request += rv;
                    length -= rv;
                }
            }

            return 0;
        }

        int32_t RecvMessage(int8_t *&response, uint32_t length)
        {
            char buffer[SYS_SOCK_SIZE];
            long timer;
#ifdef WIN32
            struct timeval exptime;
            fd_set rset;
#else
            struct pollfd fds;
            long exptime;
#endif
            int32_t rv;

            timer = 0;

            while(true)
            {
#ifdef WIN32
                FD_ZERO(&rset);
                FD_SET(m_sockfd, &rset);

                exptime.tv_sec = 1;
                exptime.tv_usec = 0;
                rv = select(m_sockfd+1, &rset, 0, 0, &exptime);
#else
                fds.fd = m_sockfd;
                fds.events = POLLRDNORM;

                exptime = 1000;
                rv = poll(&fds, 1, exptime);
#endif
                if(rv < 0)
                {
                    m_errcode = errno;
                    return SYS_SOCK_ERRIO;
                }
                else if(rv == 0)
                {
                    if(++timer >= (long)m_recvTimeout)
                    {
                        return SYS_SOCK_ERRTIMEO;
                    }
                    continue;
                }

#ifdef WIN32
                rv = recv(m_sockfd, buffer, SYS_SOCK_SIZE, 0);
#else
                do
                {
                    rv = read(m_sockfd, buffer, SYS_SOCK_SIZE);
                }
                while(rv < 0 && errno == EINTR);
#endif

                if(rv < 0)
                {
#ifndef WIN32
                    if(errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        if(timer >= (long)m_sendTimeout)
                        {
                            return SYS_SOCK_ERRTIMEO;
                        }
                        continue;
                    }
#endif
                    m_errcode = errno;
                    return SYS_SOCK_ERRIO;
                }
                else if(rv == 0)
                {
                    return SYS_SOCK_ERRFIN;
                }
                else
                {
                    ret = snprintf(response, MaxLen, "%.*s", rv, buffer); 
                    response += rv;
                    MaxLen -= rv;
                    break;
                }
            }

            return 0;
        }

    private:
        uint32_t            m_sockfd;           // 客户端socket的文件描述符（句柄）
        uint8_t             m_closed;           // 客户端是否关闭的标记
        uint8_t             m_option;           // 客户端的SYS_SOCKET_option参数
        uint32_t            m_ipaddr;           // 待连接服务器端socket的IP地址
        uint16_t            m_port;             // 待连接服务器端socket的端口号
        uint16_t            m_connectRetries;   // 客户端socket的链接超时重试次数
        uint32_t            m_connectTimeout;   // 客户端socket的链接超时参数
        uint32_t            m_sendTimeout;      // 客户端socket的发送超时参数
        uint32_t            m_recvTimeout;      // 客户端socket的接受超时参数
        int32_t             m_errcode;          // 客户端socket的访问错误编号
        void               *m_param;            // 客户端socket的可扩展参数指针
    }
}   // end of namespace
#endif //_ZJS_SOCKET_H_
