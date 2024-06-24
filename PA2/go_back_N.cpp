// Ubuntu 22.04 LTS
// g++ (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
// g++ -o go_back_N go_back_N.cpp
// 2020-17429 Kim minseo

#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace std;

struct pk_list {
    long sn;
    double gentm;
    double t_out;
    struct pk_list *link;
};
typedef struct pk_list DataQue;
DataQue *WQ_front, *WQ_rear; // packets generated, waiting to be transmitted
DataQue *TransitQ_front, *TransitQ_rear; // packets already transmitted, waiting for ACK

struct ack_list {
    long ack_n;
    double ack_rtm;
    struct ack_list *link;
};
typedef struct ack_list AckQue;
AckQue *AQ_front, *AQ_rear; // ACKs already sent, waiting for ACK to arrive

long seq_n = 0;
long transit_pknum = 0;
long next_sn = 0;
double cur_tm, next_pk_gentm;
double t_pknum = 0, t_delay = 0;

// input parameters
long N = 100;
double timeout_len;
int W;
float a, t_pk, t_pro;
float lambda, p_data, p_ack;

int K;

void initialize(void); // initialize input parameters
float uniform_random(void); // generate random number between 0 and 1
void pk_gen(double); // generate packet, insert to WQ
bool canRemove(long, long); // check if packet can be removed from TransitQ (delayed ACK due to ACK loss)
void suc_transmission(long); // succeed transmission, remove element from TransitQ and AQ
void re_transmit(void); // retransmit all packets in TransitQ (= move to the front of WQ)
void transmit_pk(void); // transmit packet in WQ, moves to TransitQ
void receive_pk(long, double); // receive, send ACK
void enque_Ack(long); // insert ack to AQ
void cur_tm_update(void);
void print_performance_measure(void);

int main() {
    initialize();

    cur_tm = -log(uniform_random())/lambda;
    pk_gen(cur_tm);
    next_pk_gentm = cur_tm + (-log(uniform_random())/lambda);

    while(t_pknum <= N) {
        while(AQ_front != NULL) {
            if(AQ_front->ack_rtm <= cur_tm)
                suc_transmission(AQ_front->ack_n);
            else
                break;
        }

        if(TransitQ_front != NULL && TransitQ_front->t_out <= cur_tm) { // timeout in first packet in TransitQ
            re_transmit();
        }

        while(next_pk_gentm <= cur_tm) { // can generate
            pk_gen(next_pk_gentm);
            next_pk_gentm += -log(uniform_random())/lambda;
        }

        if((WQ_front != NULL) && (transit_pknum < W)) {
            transmit_pk(); // transmit 하고
            receive_pk(TransitQ_rear->sn, TransitQ_rear->gentm); // receive, ACK send 이벤트까지 세팅
        }

        cur_tm_update();
    }

    print_performance_measure();

    return 0;
}

void initialize(void) {
    // load condition: W < 2a+1
    // timeout_len should be larger than 2*t_pro
    cout << "Enter the value of (W, lambda, t_pk, p_data, a, timeout_len): ";
    cin >> W >> lambda >> t_pk >> p_data >> a >> timeout_len;

    t_pro = a * t_pk;
    p_ack = p_data / 10.0; // error rate of ACK packet much smaller than data packet
    K = W+1; // minimum number of SN in Go-back-N scheme
    WQ_front = WQ_rear = NULL;
    TransitQ_front = TransitQ_rear = NULL;
    AQ_front = AQ_rear = NULL;
    srand(time(NULL));
}

float uniform_random(void) {
    return (float)rand() / (float)RAND_MAX;
}

void pk_gen(double tm) {
    DataQue *ptr;
    ptr = (DataQue*) malloc(sizeof(DataQue));
    ptr->sn = seq_n;
    ptr->gentm = tm;
    ptr->link = NULL;
    seq_n = (seq_n+1) % K;

    if(WQ_front == NULL)
        WQ_front = ptr;
    else
        WQ_rear->link = ptr;
    WQ_rear = ptr;
}

bool canRemove(long suc_pk_sn, long pk_sn) {
    if(suc_pk_sn >= W-1) {
        if(pk_sn <= suc_pk_sn && pk_sn > (suc_pk_sn - W))
            return true;
        else
            return false;
    } else {
        if(pk_sn <= suc_pk_sn || pk_sn > (K-1 - (W-suc_pk_sn-1)))
            return true;
        else
            return false;
    }  
}

void suc_transmission(long ack_sn) {
    DataQue *ptr;
    AckQue *aptr;
    long suc_pk_sn;

    suc_pk_sn = (ack_sn + K -1) % K;
    ptr = TransitQ_front;
    if(canRemove(suc_pk_sn, ptr->sn)) { // 전송 성공하면 TransitQ에서 제거
        TransitQ_front = TransitQ_front->link;
        if(TransitQ_front == NULL)
            TransitQ_rear = NULL;
        free(ptr);
        transit_pknum--;
    }
    // ACK 잘 전송되었으므로 AQ에서 제거
    aptr = AQ_front;
    AQ_front = aptr->link;
    if(AQ_front == NULL)
        AQ_rear = NULL;
    free(aptr);
}

void re_transmit(void) {
    TransitQ_rear->link = WQ_front;
    if(WQ_front == NULL)
        WQ_rear = TransitQ_rear;
    WQ_front = TransitQ_front;
    TransitQ_front = TransitQ_rear = NULL;

    transit_pknum = 0;
}

void transmit_pk(void) {
    DataQue *ptr;
    cur_tm += t_pk;

    WQ_front->t_out = cur_tm+timeout_len; // 전송 완료한 시점부터 timeout setting

    // WQ에서 제거
    ptr = WQ_front;
    WQ_front = WQ_front->link;
    if(WQ_front == NULL)
        WQ_rear = NULL;

    // TransitQ에 삽입
    if(TransitQ_front == NULL)
        TransitQ_front = ptr;
    else
        TransitQ_rear->link = ptr;
    TransitQ_rear = ptr;
    ptr->link = NULL;
    transit_pknum++;
}

void receive_pk(long seqn, double gtm) {
    if(uniform_random() > p_data) { // packet not lost
        if(next_sn == seqn) {
            t_delay += cur_tm + t_pro - gtm;
            t_pknum++;
            next_sn = (next_sn+1) % K;
            if(uniform_random() > p_ack) // ACK packet not lost
                enque_Ack(next_sn); 
        } else { // ACK가 누락되어 duplicate packet 수신한 것, ACK 재전송
            if(uniform_random() > p_ack) // ACK packet not lost
                enque_Ack(next_sn);
        }
    }
}

void enque_Ack(long seqn) {
    AckQue *ack_ptr;

    ack_ptr = (AckQue*) malloc(sizeof(AckQue));
    ack_ptr->ack_n = seqn;
    ack_ptr->ack_rtm = cur_tm + 2*t_pro;
    ack_ptr->link = NULL;

    if(AQ_front == NULL)
        AQ_front = ack_ptr;
    else
        AQ_rear->link = ack_ptr;
    AQ_rear = ack_ptr;
}

void cur_tm_update(void) {
    double tm;

    if(WQ_front != NULL && (transit_pknum < W)) { // 이미 generate되어 전송 기다리는 패킷이 있다면 유지 (transmit_pk()에서 전송하면서 cur_tm update)
        return;
    } else {
        if(AQ_front != NULL && AQ_front->ack_rtm < next_pk_gentm)
            tm = AQ_front->ack_rtm;
        else
            tm = next_pk_gentm;

        if(TransitQ_front != NULL && TransitQ_front->t_out < tm)
            tm = TransitQ_front->t_out;
        
        // ACK 송신측 도착 시간, 다음 패킷 생성 시간, TransitQ의 timeout 시간 중 가장 빠른 시간 선택
        if(tm > cur_tm) cur_tm = tm;
    }
}

void print_performance_measure(void) {
    double util;
    double m_delay;

    m_delay = t_delay/t_pknum;
    util = (t_pknum * t_pk) / cur_tm;

    printf("+--------------------------------------------------+\n");
    printf("|             packet delay |           utilization |\n");
    printf("+--------------------------------------------------+\n");
    printf("| %24.3f | %21.3f |\n", m_delay, util);
    printf("+--------------------------------------------------+\n");
}