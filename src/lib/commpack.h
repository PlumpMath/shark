#ifndef __COMMPACK_H__
#define __COMMPACK_H__

struct commpack
{
    unsigned char *buff;
    int pos;                //ָ��body�Ŀ���λ��
    int len;                //�����������İ���
};

int commpack_write_data(struct commpack *pack, void *data, int len);
int write_byte(struct commpack *pack, unsigned char param);
int write_short(struct commpack *pack, unsigned short param);
int write_int(struct commpack *pack, unsigned param);

int commpack_get_data(struct commpack *pack, void **data, int len);
int read_byte(struct commpack *pack, unsigned char *param);
int read_short(struct commpack *pack, unsigned short *param);
int read_int(struct commpack *pack, unsigned *param);

int commpack_buff_alloc(struct commpack *pack, int len);
void commpack_buff_free(struct commpack *pack);

#endif


