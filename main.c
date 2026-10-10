#include <stdio.h>
#include <stdlib.h>
#include <string.h> // String işlemleri için

// Görev durumlarını sayılarla değil isimlerle takip etmek için
typedef enum {
    HENUZ_GELMEDI = 0,
    BEKLIYOR = 1,
    DEVAM_EDIYOR = 2,
    TAMAMLANDI = 3
} GorevDurumu;

struct Gorev {
    // sabit görev verileri
    int ID;
    int GELIS_ZAMANI;
    int SURE;
    int ONCELIK;
    char GOREV_TURU[40];
    char KONUM[20];
    // değişken veriler
    GorevDurumu durum;       // görevin anlık durumu
    int kalan_sure;          // görevin bitmesine kalan süre
    int ilk_baslama_zamani;  // Bekleme süresi hesabı için
    int bitis_zamani;        // makespan ve sistemde kalma hesabı için
    char guncel_teknisyen[3];// "T1", "T2" gibi bilgileri tutacak (Boşsa "")
};

struct Teknisyen {
    char isim[3];            // "T1", "T2" vb.
    int Durum;               // calisma durmu (0/1)
    char Konum[20];          // en son çalıştığı konum
    int Gorev_ID;            // çalıştığı görevin ID'si (Yoksa 0)
    int Calisma_Suresi;      // o ana kadar toplam çalıştığı süre
    int Gecis_Suresi_Kaldi;  // konum değiştirirse 1 olacak
    int Dilimde_Kalan_Sure;  // Round Robin için, quantum sayacı
};

int main() {
    char dosya_adi[50];
    char satir[150];
    struct Gorev gorev_listesi[2000];
    int gorev_sayisi = 0;

    struct Teknisyen teknikerler[4] = {
        {"T1", 0, "", 0, 0, 0, 0},
        {"T2", 0, "", 0, 0, 0, 0},
        {"T3", 0, "", 0, 0, 0, 0},
        {"T4", 0, "", 0, 0, 0, 0}
    };

    printf("Lutfen bir dosya adi giriniz: ");
    scanf("%s", dosya_adi);

    FILE* dosya = fopen(dosya_adi, "r");

    if (dosya == NULL) {
        printf("Dosya acma islemi basarisiz oldu.\n");
    }
    else {
        int satir_no = 0;

        while (fgets(satir, 150, dosya)) {
            satir_no++;

            // (#) ve boş satırları atlamak için
            if ((satir[0] == '#') || (satir[0] == '\n')) {
                continue;
            }

            int veri_sayisi = sscanf(satir, "%d %d %d %d %s %s",
                                   &gorev_listesi[gorev_sayisi].ID,
                                   &gorev_listesi[gorev_sayisi].GELIS_ZAMANI,
                                   &gorev_listesi[gorev_sayisi].SURE,
                                   &gorev_listesi[gorev_sayisi].ONCELIK,
                                   gorev_listesi[gorev_sayisi].GOREV_TURU,
                                   gorev_listesi[gorev_sayisi].KONUM);

            // eksik alan kontrolü
            if (veri_sayisi != 6) {
                printf("Hata: %d. satir reddedildi (Neden: Eksik veya hatali format).\n", satir_no);
                continue;
            }

            // geliş zamanı kontrolü
            if (gorev_listesi[gorev_sayisi].GELIS_ZAMANI < 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz gelis zamani).\n", satir_no);
                continue;
            }

            // süre kontrolü
            if (gorev_listesi[gorev_sayisi].SURE <= 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz sure degeri).\n", satir_no);
                continue;
            }

            // öncelik kontrolü
            if ((gorev_listesi[gorev_sayisi].ONCELIK < 1) || (gorev_listesi[gorev_sayisi].ONCELIK > 5)) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz oncelik degeri).\n", satir_no);
                continue;
            }

            // tekrarlanan ID kontrolü
            int id_cakismasi = 0;
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].ID == gorev_listesi[gorev_sayisi].ID) {
                    id_cakismasi = 1;
                    break; // ID zaten varsa aramayı bırak
                }
            }
            if (id_cakismasi == 1) {
                printf("Hata: %d. satir reddedildi (Neden: Yinelenen ID).\n", satir_no);
                continue;
            }

            // kontrollerden geçtiyse başlangıç değerleri ayarlanır
            gorev_listesi[gorev_sayisi].durum = HENUZ_GELMEDI;      //sanal zaman 0 iken bütün görevler "HENUZ_GELMEDİ" durumunda
            gorev_listesi[gorev_sayisi].kalan_sure = gorev_listesi[gorev_sayisi].SURE;
            gorev_listesi[gorev_sayisi].ilk_baslama_zamani = -1; // Zaman 0'dan başladığı için, henüz başlamamış göreve "boş/atanmadı" anlamında -1 verilir.
            gorev_listesi[gorev_sayisi].bitis_zamani = -1;      //Tamamlanmamış görevin bitiş zamanı bulunmayacağı için -1 yapıyoruz.
            strcpy(gorev_listesi[gorev_sayisi].guncel_teknisyen, ""); // teknisyen bilgisi boş

            // Sayacı 1 artır, diğer görevi kaydetmeye hazır hale gel
            gorev_sayisi++;
        }

        fclose(dosya);
        printf("\nDosya okuma tamamlandi! Toplam %d gecerli gorev yuklendi.\n", gorev_sayisi);
    }

    return 0;
}
