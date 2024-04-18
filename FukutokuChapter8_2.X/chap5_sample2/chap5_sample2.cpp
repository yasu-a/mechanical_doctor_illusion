#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��
#include <thread>

static bool stop = false;
static int current_index = 0;
static int values[4] = { 0 };

void send_value(CSerial *cserial, int i, uint16_t value) {
    int result;
    for (;;) {
        result = cserial->ReceiveSerialData(128);
        if (result < 0) {
            exit(result);
        }

        unsigned char *buf = cserial->m_senddata;
        buf[0] = 0xaa;
        buf[1] = 0x55;
        buf[2] = i + 1;
        buf[3] = value >> 8;
        buf[4] = value & 0xff;
        cserial->SendSerialData(5);
        result = cserial->ReceiveSerialData(32);
        if (result < 0) {
            exit(result);
        }
        if (cserial->m_receivedata[0] == 'O' && cserial->m_receivedata[1] == 'K') {
            break;
        }
    }
}

void serial_worker() {
    CSerial *cserial;
    cserial = new CSerial; //Cserial�N���X���擾
    cserial->MakeBuffer(1024, 1024); //���M�p�f�[�^��1byte�A��M�p�f�[�^��1byte�p�ӂ���B
    cserial->SetSerialPortName(TEXT("COM5")); //�p�\�R���̃V���A���|�[�g��ݒ肷��B�����̃p�\�R���̃f�o�C�X�}�l�[�W���Ŋm�F���邱�ƁB
    cserial->OpenSerialPort(); //�V���A���|�[�g���I�[�v������B
    cserial->SetSerialPort(9600, 1024, 1024); // �{�[���C�g�̐ݒ�B�����Ń{�[���C�g��2400�ɂ��Ă���B
    cserial->SerialPortBufferClear(); //�V���A���|�[�g�̑���MFIFO���������N���A����B

    int prev_values[4] = { -1, -1, -1, -1 };

    while (!stop) {
        bool updated = false;
        for (int i = 0; i < 4; i++) {
            if (prev_values[i] != values[i]) {
                updated = true;
            }
        }
        if (!updated) {
            Sleep(50);
            continue;
        }

        for (int i = 0; i < 4; i++) {
            int value = values[i];
            if (prev_values[i] != value) {
                send_value(cserial, i, (uint16_t)value);
                prev_values[i] = value;
            }
        }
    }

    cserial->CloseSerialPort();
    delete cserial; //CSerial�N���X���J��
}

void print_state(int current_index) {
    putchar('\r');
    for (int i = 0; i < 4; i++) {
        char mark = current_index == i ? '*' : ' ';
        printf("[%c] LED%d %6.2f%% ", mark, i + 1, (float)values[i] / (float)0xffff * 100);
    }
}

void print_worker() {
    while (!stop) {
        print_state(current_index);
        Sleep(30);
    }
}

void main(void)
{
    unsigned long long last_time_move = 0;
    int update_flag = 1;

    std::thread serial_thread(serial_worker);
    std::thread print_thread(print_worker);

    for (unsigned int i = 0; ; i++) {
        if (GetKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        int delta = (GetKeyState(VK_LEFT) & 0x8000 ? -1 : 0) + (GetKeyState(VK_RIGHT) & 0x8000 ? +1 : 0);
        if (delta != 0) {
            if (GetTickCount64() - last_time_move > 400) {
                current_index = (current_index + 4 + delta) % 4;
                last_time_move = GetTickCount64();
            }
        }

        int step = (GetKeyState(VK_SHIFT) & 0x8000 ? 1 : 10) * (GetKeyState(VK_CONTROL) & 0x8000 ? 1 : 100);
        int sign = GetKeyState(VK_UP) & 0x8000 ? 1 : (GetKeyState(VK_DOWN) & 0x8000 ? -1 : 0);
        values[current_index] = min(max(values[current_index] + sign * step, 0), 0xffff);

        Sleep(30);
    }

    stop = true;
    serial_thread.join();
    print_thread.join();
}