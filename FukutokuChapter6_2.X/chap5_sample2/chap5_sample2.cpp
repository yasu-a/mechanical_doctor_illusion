#include <stdio.h>
#pragma warning( disable : 4996 )
#include "cserial.h" //�V���A���ʐM�p�N���X�̃w�b�_�t�@�C��

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

    while (1) {
        int n_send = 0;
        printf("COMMAND ?> ");
        cserial->m_senddata[n_send++] = scan_int_as_char();
        if (cserial->m_senddata[0] <= 2) {  // command 0, 1, 2 requires parameter
            printf("ARG1 ?> ");
            cserial->m_senddata[n_send++] = scan_int_as_char();
        }
        cserial->SendSerialData(n_send);

        int n_recv = cserial->ReceiveSerialData(cserial->m_senddata[0] <= 2 ? 0 : 1);
        if (n_recv < 0) {
            break;
        }
        printf("RECV %d: ", n_recv);
        for (int i = 0; i < n_recv; i++) {
            printf("%02x ", (unsigned int)cserial->m_receivedata[i]);
        }
        putchar('\n');
    }

    cserial->CloseSerialPort(); //�V���A���|�[�g���N���[�Y����B
    delete cserial; //CSerial�N���X���J��
}