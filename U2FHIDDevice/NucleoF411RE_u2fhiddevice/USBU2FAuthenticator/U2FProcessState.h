#ifndef U2FPROCESSSTATE_H_
#define U2FPROCESSSTATE_H_

//
// 処理ステータスを管理する変数
//
typedef enum {
    U2FPS_NONE,
    U2FPS_RECV_REQ,
    U2FPS_RECV_REQ_DONE,
    U2FPS_XFER_REQ,
    U2FPS_XFER_REQ_DONE,
    U2FPS_XFER_RES,
    U2FPS_HELPER_TIMEOUT
} U2F_PROCESS_STATE;

void u2f_process_state_set(U2F_PROCESS_STATE _state);
void u2f_process_state_main(void);
void u2f_process_state_on_receive_response(void);

#endif // U2FPROCESSSTATE_H_
