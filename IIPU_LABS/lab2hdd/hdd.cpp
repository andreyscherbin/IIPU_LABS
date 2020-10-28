#include <linux/hdreg.h>
#include <sys/types.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <bits/stdc++.h>
#include <sys/statvfs.h>

using namespace std;

//функция преобразования строки типа char* в строку типа string
string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

//дальше идут две функции для разбития строки на отдельные слова через
//любой разделитель
template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int main(void) {
    static struct hd_driveid hard_driver_pasport;
    int hard_driver_handler;
    struct statvfs used_free_total;

    hard_driver_handler=open("/dev/sda",O_RDONLY);
       if(!hard_driver_handler) {
   	perror("ERROR:OPEN");
    _exit(2);
       }

if (ioctl(hard_driver_handler, HDIO_GET_IDENTITY, &hard_driver_pasport)) {
    perror("ERROR: HDIO_GET_IDENTITY");
    _exit(1);
  }

  //4 строки кода, которые преобразуют массив char* в string и разбивают
  //string на отдельные слова, после выбираем интересующее нас слово, а именно модель жесткого диска
  //отбрасывая слово производителя
  int model_size = sizeof(hard_driver_pasport.model) / sizeof(char);
  string model = convertToString((char*)hard_driver_pasport.model, model_size);
  std::vector<std::string> vector_producer_model = split(model, ' ');
  cout << "Модель - " << vector_producer_model[1] << endl;
  cout << "Изготовитель - " << vector_producer_model[0]  << endl;
  cout << "Серийный номер - " << hard_driver_pasport.serial_no << endl;
  cout << "Версия прошивки - ";
  //костыль для вывода массива fw_rev из 8 байт, через поток данные слипаются со следующими в структуре
  for(int i =0;i<8;i++){
    printf("%c",hard_driver_pasport.fw_rev[i]);
  }
  cout << endl;

  printf("\nСведения о памяти:\n");
  //используя функция смещения lseek, смещаемся от начала диска в конец и получаем его размер
  off_t all_space = lseek(hard_driver_handler, 0, SEEK_END);
  if (all_space == (off_t)-1) { perror("ERROR:LSEEK");}
  printf("Всего  - %lu (B) (%lu GB)\n",all_space,all_space/1024/1024/1024);
  //массив из 4-ых основных файловых систем
  const char* path[3] = {"/home","/","/dev"};
  off_t free_space = 0;
  for(int i = 0; i < 4; i++){
  //функция для подсчета всего-свободно-занятно памяти на логических дисках(файловых системах)
  if((statvfs(path[i],&used_free_total)) < 0 ) {
                    //cout << "\nFailed to stat\n";
            } else {
                    //cout << "free space on " << path[i] << " " << (used_free_total.f_bsize*used_free_total.f_bavail)/1024/1024/1024 << endl;
                    free_space+=(used_free_total.f_bsize*used_free_total.f_bavail);
            }
          }
  printf("Свободно  - %lu (B) (%lu GB)\n",free_space,free_space/1024/1024/1024);
  printf("Занято  - %lu (B) (%lu GB)\n",all_space-free_space,(all_space-free_space)/1024/1024/1024);

  //Слово в паспорте под номером 222 определяет тип интерфейса - параллельный или последовательный
  printf("Тип интерфейса - %s%s", (hard_driver_pasport.words206_254[16] & (1 << 12)) ? "SATA" : "PATA", "\n");

  printf("\nPIO:\n");
  printf(" Поддерживаются:\n");
  printf("  [%s%s", (hard_driver_pasport.eide_pio_modes & 1) ? "+" : "-", "] PIO 3\n");
  printf("  [%s%s", (hard_driver_pasport.eide_pio_modes & 2) ? "+" : "-", "] PIO 4\n");

  printf("\nMultiword DMA:\n");
  printf(" Поддерживаются:\n");
  printf("  [%s%s", (hard_driver_pasport.dma_mword & 1) ? "+" : "-", "] MWDMA 0\n");
  printf("  [%s%s", (hard_driver_pasport.dma_mword & 2) ? "+" : "-", "] MWDMA 1\n");
  printf("  [%s%s", (hard_driver_pasport.dma_mword & 4) ? "+" : "-", "] MWDMA 2\n");

  printf("\nUltra DMA:\n");
  printf(" Поддерживаются:\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & 1) ? "+" : "-", "] UDMA Mode 0\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 1)) ? "+" : "-", "] UDMA Mode 1\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 2)) ? "+" : "-", "] UDMA Mode 2\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 3)) ? "+" : "-", "] UDMA Mode 3\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 4)) ? "+" : "-", "] UDMA Mode 4\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 5)) ? "+" : "-", "] UDMA Mode 5\n");
  printf("  [%s%s", (hard_driver_pasport.dma_ultra & (1 << 6)) ? "+" : "-", "] UDMA Mode 6\n");

close(hard_driver_handler);
}
