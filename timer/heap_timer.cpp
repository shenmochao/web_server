#include "heap_timer.h"
#include "../http/http_conn.h"

sort_timer_lst::sort_timer_lst() {
    timer_heap.reserve(64);
}

sort_timer_lst::~sort_timer_lst() {
    for (util_timer* timer : timer_heap) {
        delete timer;
    }
    timer_heap.clear();
}

int sort_timer_lst::find_timer(util_timer *timer) {
    auto it = lower_bound(timer_heap.begin(), timer_heap.end(), timer,
        [](const util_timer* lhs, const util_timer* rhs) {
            return lhs->expire < rhs->expire;
        });
    while (it != timer_heap.end() && (*it)->expire == timer->expire && *it != timer)
    {
        ++it;
    }

    if (it != timer_heap.end() && *it == timer)
    {
        return it - timer_heap.begin();
    }
    return -1;
}

void sort_timer_lst::heapify_up(int index) {
    while (index > 0) {
        int parent = (index-1)/2;
        if (timer_heap[index]->expire >= timer_heap[parent]-> expire) {
            break;
        }
        swap(timer_heap[index], timer_heap[parent]);
        index = parent;
    }
}

void sort_timer_lst::heapify_down(int index) {
    int n = timer_heap.size();
    while(index > 0) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        if (timer_heap[smallest]->expire > timer_heap[right]-> expire) {
            smallest = right;
        }
        if (timer_heap[smallest]->expire > timer_heap[left]-> expire) {
            smallest = left;
        }
        if (smallest == index)
        {
            break;
        }
        swap(timer_heap[index], timer_heap[smallest]);
        index = smallest;
    }
}

void sort_timer_lst::add_timer(util_timer *timer) {
    timer_heap.push_back(timer);
    heapify_up(timer_heap.size() - 1);
}

void sort_timer_lst::adjust_timer(util_timer *timer) {
    int index = find_timer(timer);
    if (index != -1)
    {
        heapify_up(index);
        heapify_down(index);
    }
}

void sort_timer_lst::del_timer(util_timer *timer) {
    int index = find_timer(timer);
    if (index != -1)
    {
        std::swap(timer_heap[index], timer_heap.back());
        timer_heap.pop_back();
        if (index < timer_heap.size())
        {
            heapify_up(index);
            heapify_down(index);
        }
    }
}

void sort_timer_lst::tick() {
    time_t cur = time(NULL);
    while(!timer_heap.empty()) {
        util_timer *timer = timer_heap.front();
        if(timer->expire > cur) {
            break;
        }
        if (timer->cb_func)
        {
            timer->cb_func(timer->user_data);
        }
        del_timer(timer);
    }
}

void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    m_timer_lst.tick();
    alarm(m_TIMESLOT);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}