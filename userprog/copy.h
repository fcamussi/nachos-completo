#ifndef COPY_H
#define COPY_H



static void ReadMem(int addr, int size, int *value);
static void WriteMem(int addr, int size, int value);

void CopyFromUser(int addr, int size, char *buffer);
void CopyToUser(int addr, int size, char *buffer);
void CopyStringFromUser(int addr, int size, char *buffer);


class Arguments
{
  public:
    Arguments();
    ~Arguments();
    void CopyFromUser(int addr);
    int CopyToUserStack(int addr);
    int GetArgc() { return argc; }
  private:
    int argc;
    char **argv;
};



#endif // COPY_H

