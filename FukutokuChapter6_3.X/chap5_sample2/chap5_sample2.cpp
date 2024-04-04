#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��
#include <cstdint>

char scan_int_as_char(void) {
    int value;
    scanf("%d", &value);
    return (char)value;
}

void main(void)
{
    int data = 0;
    CSerial *cserial; //CSerial�N���X�̃|�C���^�̐錾
    cserial = new CSerial; //Cserial�N���X���擾
    cserial->MakeBuffer(32, 32); //���M�p�f�[�^��1byte�A��M�p�f�[�^��1byte�p�ӂ���B
    cserial->SetSerialPortName(TEXT("COM5")); //�p�\�R���̃V���A���|�[�g��ݒ肷��B�����̃p�\�R���̃f�o�C�X�}�l�[�W���Ŋm�F���邱�ƁB
    cserial->OpenSerialPort(); //�V���A���|�[�g���I�[�v������B
    cserial->SetSerialPort(9600, 1024, 1024); // �{�[���C�g�̐ݒ�B�����Ń{�[���C�g��2400�ɂ��Ă���B
    cserial->SerialPortBufferClear(); //�V���A���|�[�g�̑���MFIFO���������N���A����B
    //�}�C�i�X�̐��l����͂���ƃv���O�����I��
    // while (data >= 0)
    // {
    //     printf("dsPIC�ɑ��M�������f�[�^�����ĉ������B\n");
    //     scanf("%d", &data);
    //     printf("SEND: %d\n", data);
    //     cserial->m_senddata[0] = (unsigned char)data; //���M�p�f�[�^����
    //     cserial->SendSerialData(1); //�p�\�R������dsPIC��1byte�𑗐M
    //     int recv_size = cserial->ReceiveSerialData(1); //�p�\�R���ɗ��Ă���V���A���f�[�^��1byte��M
    //     if (recv_size > 0) {
    //         printf("RECV %d\n", cserial->m_receivedata[0]); //��M�����f�[�^��\��
    //     }
    // }

    char buf[32] = { 0 };
    while (1) {
        printf("\r                               \r%s", buf);

        int n_recv = cserial->ReceiveSerialData(1);
        if (n_recv < 0) {
            break;
        }
        if (n_recv == 0) {
            continue;
        }
        if (n_recv != 1 || cserial->m_receivedata[0] != 0x55) {
            sprintf(buf, "NO DATA %02x\r", (unsigned char)cserial->m_receivedata[0]);
            continue;
        }

        n_recv = cserial->ReceiveSerialData(1);
        if (n_recv < 0) {
            break;
        }
        if (n_recv == 0) {
            continue;
        }
        if (n_recv != 1 || cserial->m_receivedata[0] != 0xaa) {
            sprintf(buf, "NO DATA %02x\r", (unsigned char)cserial->m_receivedata[0]);
            continue;
        }

        n_recv = cserial->ReceiveSerialData(2);
        if (n_recv != 2) {
            sprintf(buf, "NO DATA\r");
            continue;
        }
        uint8_t lower = cserial->m_receivedata[0];
        uint8_t upper = cserial->m_receivedata[1];
        uint16_t u16 = (uint16_t)upper << 8 | (uint16_t)lower;
        sprintf(buf, "%02x%02x %+d", upper, lower, *(int16_t *)&u16);
    }

    cserial->CloseSerialPort(); //�V���A���|�[�g���N���[�Y����B
    delete cserial; //CSerial�N���X���J��
}