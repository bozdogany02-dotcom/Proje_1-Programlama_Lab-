#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Görev durumları
typedef enum {
    HENUZ_GELMEDI = 0,
    BEKLIYOR = 1,
    DEVAM_EDIYOR = 2,
    TAMAMLANDI = 3
} GorevDurumu;

const char* durum_metinleri[] = {"Henuz Gelmedi", "Bekliyor", "Devam Ediyor", "Tamamlandi"};

struct Gorev {
    int ID;
    int GELIS_ZAMANI;
    int SURE;
    int ONCELIK;
    char GOREV_TURU[40];
    char KONUM[20];

    GorevDurumu durum;
    int kalan_sure;
    int ilk_baslama_zamani;
    int bitis_zamani;
    char guncel_teknisyen[5];
    int rr_sirasi; // Round Robin algoritması için kuyruğa giriş sırası
};

struct Teknisyen {
    char isim[5];
    int Durum;      //0: Boş, 1: Meşgul
    char Konum[20];
    int Gorev_ID;
    int Calisma_Suresi;
    int Gecis_Suresi_Kaldi; // Konum değişim cezası takip değişkeni
    int Dilimde_Kalan_Sure; // Round Robin kuantum sayacı
};

// Sistemi zaman=0 anındaki başlangıç durumuna getirir
void simulasyonu_sifirla(struct Gorev gorev_listesi[], int gorev_sayisi, struct Teknisyen teknikerler[], int *sanal_zaman_ptr, int *siramatik_ptr) {
    *sanal_zaman_ptr = 0;
    *siramatik_ptr = 1;

    // Görevleri başlangıç durumuna getir
    for (int i = 0; i < gorev_sayisi; i++) {
        gorev_listesi[i].durum = HENUZ_GELMEDI;
        gorev_listesi[i].kalan_sure = gorev_listesi[i].SURE;
        gorev_listesi[i].ilk_baslama_zamani = -1;
        gorev_listesi[i].bitis_zamani = -1;
        strcpy(gorev_listesi[i].guncel_teknisyen, "");
        gorev_listesi[i].rr_sirasi = 0;
    }

    // Teknisyen durumlarını ve istatistiklerini sıfırla
    for (int i = 0; i < 4; i++) {
        teknikerler[i].Durum = 0;
        strcpy(teknikerler[i].Konum, "");
        teknikerler[i].Gorev_ID = 0;
        teknikerler[i].Calisma_Suresi = 0;
        teknikerler[i].Gecis_Suresi_Kaldi = 0;
        teknikerler[i].Dilimde_Kalan_Sure = 0;
    }
}

// 4. Seçenek: Özel Algoritma
int benim_algoritmam(struct Gorev gorev_listesi[], int gorev_sayisi, struct Teknisyen teknikerler[], int t_index) {
    int ayni_konum_secilen = -1;
    int farkli_bos_konum_secilen = -1;
    int en_uzun_secilen = -1;

    for (int j = 0; j < gorev_sayisi; j++) {
        if (gorev_listesi[j].durum == BEKLIYOR) {

            // Aynı konumdaki görevleri kontrol et (Ceza yok)
            if (strcmp(teknikerler[t_index].Konum, gorev_listesi[j].KONUM) == 0 && strcmp(teknikerler[t_index].Konum, "") != 0) {
                if (ayni_konum_secilen == -1 || gorev_listesi[j].SURE > gorev_listesi[ayni_konum_secilen].SURE) {
                    ayni_konum_secilen = j;
                }
            }

            // Diğer teknisyenlerin bulunmadığı konumları kontrol et
            int konum_mesgul = 0;
            for (int k = 0; k < 4; k++) {
                if (k != t_index && teknikerler[k].Durum == 1 && strcmp(teknikerler[k].Konum, gorev_listesi[j].KONUM) == 0) {
                    konum_mesgul = 1;
                    break;
                }
            }
            // Diğer teknisyenlerin çalışmadığı boş konumları kontrol et (Yığılmayı önle)
            if (konum_mesgul == 0) {
                if (farkli_bos_konum_secilen == -1 || gorev_listesi[j].SURE > gorev_listesi[farkli_bos_konum_secilen].SURE) {
                    farkli_bos_konum_secilen = j;
                }
            }

            // Hiçbiri uymazsa en uzun süreli görevi seç
            if (en_uzun_secilen == -1 || gorev_listesi[j].SURE > gorev_listesi[en_uzun_secilen].SURE) {
                en_uzun_secilen = j;
            }
        }
    }

    // Belirlenen hiyerarşiye göre uygun görevin indeksini döndür
    if (ayni_konum_secilen != -1) return ayni_konum_secilen;
    else if (farkli_bos_konum_secilen != -1) return farkli_bos_konum_secilen;
    else return en_uzun_secilen;
}

// Boş teknisyen için seçilen algoritmaya uygun görevin indeksini bulur
int siradaki_gorevi_bul(struct Gorev gorev_listesi[], int gorev_sayisi, int secili_algoritma, struct Teknisyen teknikerler[], int t_index) {
    int secilen_index = -1;

    if (secili_algoritma == 1 || secili_algoritma == 2 || secili_algoritma == 3) {
        for (int j = 0; j < gorev_sayisi; j++) {
            if (gorev_listesi[j].durum == BEKLIYOR) {
                if (secilen_index == -1) {
                    secilen_index = j;
                    continue;
                }

                if (secili_algoritma == 1) { // Shortest Job First
                    if (gorev_listesi[j].SURE < gorev_listesi[secilen_index].SURE) secilen_index = j;
                    else if (gorev_listesi[j].SURE == gorev_listesi[secilen_index].SURE) {
                        if (gorev_listesi[j].GELIS_ZAMANI < gorev_listesi[secilen_index].GELIS_ZAMANI) secilen_index = j;
                        else if (gorev_listesi[j].GELIS_ZAMANI == gorev_listesi[secilen_index].GELIS_ZAMANI) {
                            if (gorev_listesi[j].ID < gorev_listesi[secilen_index].ID) secilen_index = j;
                        }
                    }
                }
                else if (secili_algoritma == 2) { // Priority
                    if (gorev_listesi[j].ONCELIK < gorev_listesi[secilen_index].ONCELIK) secilen_index = j;
                    else if (gorev_listesi[j].ONCELIK == gorev_listesi[secilen_index].ONCELIK) {
                        if (gorev_listesi[j].GELIS_ZAMANI < gorev_listesi[secilen_index].GELIS_ZAMANI) secilen_index = j;
                        else if (gorev_listesi[j].GELIS_ZAMANI == gorev_listesi[secilen_index].GELIS_ZAMANI) {
                            if (gorev_listesi[j].ID < gorev_listesi[secilen_index].ID) secilen_index = j;
                        }
                    }
                }
                else if (secili_algoritma == 3) { // Round Robin
                    if (gorev_listesi[j].rr_sirasi < gorev_listesi[secilen_index].rr_sirasi) secilen_index = j;
                }
            }
        }
    }
    else if (secili_algoritma == 4) { // Özel Algoritma
        secilen_index = benim_algoritmam(gorev_listesi, gorev_sayisi, teknikerler, t_index);
    }

    return secilen_index;
}

// Zamanı 1 birim ilerletir ve gerekli atama işlemlerini yapar
void zaman_ilerletme_ve_atama(struct Gorev gorev_listesi[], int gorev_sayisi, struct Teknisyen teknikerler[], int secili_algoritma, int quantum_degeri, int *sanal_zaman_ptr, int *siramatik_ptr) {

    // Kuantumu dolan görevlerin indekslerini tutar
    int kuantumu_dolanlar[4] = {-1, -1, -1, -1};

    // ADIM 1: Devam eden görevleri kontrol et
    for (int i = 0; i < 4; i++) {
        if (teknikerler[i].Durum == 1) {
            int g_id = teknikerler[i].Gorev_ID;
            for (int j = 0; j < gorev_sayisi; j++) {
                if (gorev_listesi[j].ID == g_id) {
                    if (gorev_listesi[j].kalan_sure == 0) {
                        gorev_listesi[j].durum = TAMAMLANDI;
                        gorev_listesi[j].bitis_zamani = (*sanal_zaman_ptr);
                        teknikerler[i].Durum = 0;
                        teknikerler[i].Gorev_ID = 0;
                    }
                    else if (secili_algoritma == 3 && teknikerler[i].Dilimde_Kalan_Sure == quantum_degeri && teknikerler[i].Gecis_Suresi_Kaldi == 0) {
                        kuantumu_dolanlar[i] = j;
                    }
                    break;
                }
            }
        }
    }

    // ADIM 2: Yeni gelen görevleri bekleme kuyruğuna al (ID sırasına göre)
    int eklenecek_var_mi = 1;
    while (eklenecek_var_mi) {
        eklenecek_var_mi = 0;
        int min_id_index = -1;

        for (int i = 0; i < gorev_sayisi; i++) {
            if (gorev_listesi[i].durum == HENUZ_GELMEDI && gorev_listesi[i].GELIS_ZAMANI <= (*sanal_zaman_ptr)) {
                if (min_id_index == -1 || gorev_listesi[i].ID < gorev_listesi[min_id_index].ID) {
                    min_id_index = i;
                    eklenecek_var_mi = 1;
                }
            }
        }

        if (eklenecek_var_mi && min_id_index != -1) {
            gorev_listesi[min_id_index].durum = BEKLIYOR;
            gorev_listesi[min_id_index].rr_sirasi = (*siramatik_ptr)++;
        }
    }

    // ADIM 3: Kuantumu dolan görevleri kuyruğun sonuna al
    for (int i = 0; i < 4; i++) {
        if (kuantumu_dolanlar[i] != -1) {
            int g_index = kuantumu_dolanlar[i];
            gorev_listesi[g_index].durum = BEKLIYOR;
            gorev_listesi[g_index].rr_sirasi = (*siramatik_ptr)++;
            teknikerler[i].Durum = 0;
            teknikerler[i].Gorev_ID = 0;
        }
    }

    // ADIM 4: Boş teknisyenlere yeni atama yap
    for (int i = 0; i < 4; i++) {
        if (teknikerler[i].Durum == 0) {
            int secilen_index = siradaki_gorevi_bul(gorev_listesi, gorev_sayisi, secili_algoritma, teknikerler, i);

            if (secilen_index != -1) {
                // Görevin ilk başlama zamanını kaydet
                if (gorev_listesi[secilen_index].ilk_baslama_zamani == -1) {
                    gorev_listesi[secilen_index].ilk_baslama_zamani = (*sanal_zaman_ptr);
                }

                // Konum değişim cezası kontrolü
                if (strcmp(teknikerler[i].Konum, gorev_listesi[secilen_index].KONUM) != 0 && strcmp(teknikerler[i].Konum, "") != 0) {
                    teknikerler[i].Gecis_Suresi_Kaldi = 1;
                } else {
                    teknikerler[i].Gecis_Suresi_Kaldi = 0;
                }

                // Görevi teknisyene ata
                gorev_listesi[secilen_index].durum = DEVAM_EDIYOR;
                strcpy(gorev_listesi[secilen_index].guncel_teknisyen, teknikerler[i].isim);

                teknikerler[i].Durum = 1;
                teknikerler[i].Gorev_ID = gorev_listesi[secilen_index].ID;
                strcpy(teknikerler[i].Konum, gorev_listesi[secilen_index].KONUM);
                teknikerler[i].Dilimde_Kalan_Sure = 0;
            }
        }
    }

    // ADIM 5: Devam eden görevlerin sürelerini güncelle
    for (int i = 0; i < 4; i++) {
        if (teknikerler[i].Durum == 1) {
            // Teknisyen geçişteyse sadece ceza süresi azalır
            if (teknikerler[i].Gecis_Suresi_Kaldi > 0) {
                teknikerler[i].Gecis_Suresi_Kaldi--;
            }
            // Geçiş bittiyse görev süresi azalır
            else {
                int g_id = teknikerler[i].Gorev_ID;
                for (int j = 0; j < gorev_sayisi; j++) {
                    if (gorev_listesi[j].ID == g_id) {
                        gorev_listesi[j].kalan_sure--;
                        break;
                    }
                }
                teknikerler[i].Dilimde_Kalan_Sure++;
                teknikerler[i].Calisma_Suresi++;
            }
        }
    }

    // Sanal zamanı artır
    (*sanal_zaman_ptr)++;
}

int main() {
    int sanal_zaman = 0;
    int siramatik = 1;

    char dosya_adi[50];
    char satir[150];
    struct Gorev gorev_listesi[2000];
    int gorev_sayisi=0;

    struct Teknisyen teknikerler[4]={
        {"T1",0,"",0,0,0,0},
        {"T2",0,"",0,0,0,0},
        {"T3",0,"",0,0,0,0},
        {"T4",0,"",0,0,0,0}
    };

    printf("Lutfen bir dosya adi giriniz: ");
    scanf("%s",dosya_adi);

    FILE* dosya;
    dosya=fopen(dosya_adi,"r");
    if(dosya==NULL){
        printf("Dosya acma islemi basarisiz oldu.");
    }
    else{
        int satir_no = 0;

        while (fgets(satir, 150, dosya)) {
            satir_no++;

            // Yorum ve boş satırları atla
            if ((satir[0] == '#') || (satir[0] == '\n')) continue;

            int veri_sayisi = sscanf(satir, "%d %d %d %d %s %s",
                                     &gorev_listesi[gorev_sayisi].ID,
                                     &gorev_listesi[gorev_sayisi].GELIS_ZAMANI,
                                     &gorev_listesi[gorev_sayisi].SURE,
                                     &gorev_listesi[gorev_sayisi].ONCELIK,
                                     gorev_listesi[gorev_sayisi].GOREV_TURU,
                                     gorev_listesi[gorev_sayisi].KONUM);

            // Veri doğrulama işlemleri
            if (veri_sayisi != 6) {
                printf("Hata: %d. satir reddedildi (Neden: Eksik veya hatali format).\n", satir_no);
                continue;
            }
            if (gorev_listesi[gorev_sayisi].GELIS_ZAMANI < 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz gelis zamani).\n", satir_no);
                continue;
            }
            if (gorev_listesi[gorev_sayisi].SURE <= 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz sure degeri).\n", satir_no);
                continue;
            }
            if ((gorev_listesi[gorev_sayisi].ONCELIK < 1) || (gorev_listesi[gorev_sayisi].ONCELIK > 5)) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz oncelik degeri).\n", satir_no);
                continue;
            }

            int id_cakismasi = 0;
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].ID == gorev_listesi[gorev_sayisi].ID) {
                    id_cakismasi = 1;
                    break;
                }
            }
            if (id_cakismasi == 1) {
                printf("Hata: %d. satir reddedildi (Neden: Yinelenen ID).\n", satir_no);
                continue;
            }

            // Geçerli görevi listeye ekle
            gorev_listesi[gorev_sayisi].durum = HENUZ_GELMEDI;
            gorev_listesi[gorev_sayisi].kalan_sure = gorev_listesi[gorev_sayisi].SURE;
            gorev_listesi[gorev_sayisi].ilk_baslama_zamani = -1;
            gorev_listesi[gorev_sayisi].bitis_zamani = -1;
            strcpy(gorev_listesi[gorev_sayisi].guncel_teknisyen, "");
            gorev_listesi[gorev_sayisi].rr_sirasi = 0;

            gorev_sayisi++;
        }
        fclose(dosya);
        printf("\nDosya okuma tamamlandi! Toplam %d gecerli gorev yuklendi.\n", gorev_sayisi);
    }

    int secili_algoritma = 0;
    int quantum_degeri = 0;
    int secim;

    while(1) {
        printf("\n-------------------------------------------------------\n");
        printf("GUNCEL ZAMAN: %d\n", sanal_zaman);
        printf("SECILI ALGORITMA: ");
        if(secili_algoritma == 1) printf("Shortest Job\n");
        else if(secili_algoritma == 2) printf("Priority\n");
        else if(secili_algoritma == 3) printf("Round Robin (Quantum: %d)\n", quantum_degeri);
        else if(secili_algoritma == 4) printf("Kendi Algoritmam\n");
        else printf("Henuz secilmedi\n");
        printf("-------------------------------------------------------\n");
        printf("1. Algoritma sec veya degistir\n");
        printf("2. Tum gorevleri listele\n");
        printf("3. Gorevleri durumuna gore listele\n");
        printf("4. Gorev ID ile sorgula\n");
        printf("5. Teknisyenlerin durumunu goster\n");
        printf("6. Zamani 1 birim ilerlet\n");
        printf("7. Zamani N birim ilerlet\n");
        printf("8. Belirli bir zamana git\n");
        printf("9. Istatistikleri goster\n");
        printf("10. Simulasyonu sifirla\n");
        printf("11. Cikis\n");
        printf("-------------------------------------------------------\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);

        switch(secim){
            case 1:
                printf("\n--- Algoritma Secimi ---\n");
                printf("1- Shortest Job\n");
                printf("2- Priority\n");
                printf("3- Round Robin\n");
                printf("4- Kendi Algoritmam\n");
                printf("Seciminiz: ");
                scanf("%d", &secili_algoritma);

                if(secili_algoritma == 3) {
                    printf("Round Robin icin zaman dilimi (quantum) giriniz: ");
                    scanf("%d", &quantum_degeri);
                }

                // Yeni algoritma seçimi sonrası sistemi sıfırla
                simulasyonu_sifirla(gorev_listesi, gorev_sayisi, teknikerler, &sanal_zaman, &siramatik);
                printf("Algoritma secildi. Zaman ve onceki durumlar sifirlandi.\n");
                break;

            case 2:
                printf("\n--- TUM GOREVLERIN LISTESI ---\n");
                printf("%-6s %-14s %-12s %-9s %-15s %-18s %-12s\n",
                       "ID", "Gelis Zamani", "Toplam Sure", "Oncelik", "Durum", "Guncel Teknisyen", "Kalan Sure");
                printf("----------------------------------------------------------------------------------------------\n");

                for (int i = 0; i < gorev_sayisi; i++) {
                    char teknisyen_gosterim[5];
                    // Boş teknisyen alanı için "-" yazdır
                    if (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0) {
                        strcpy(teknisyen_gosterim, "-");
                    } else {
                        strcpy(teknisyen_gosterim, gorev_listesi[i].guncel_teknisyen);
                    }

                    printf("%-6d %-14d %-12d %-9d %-15s %-18s %-12d\n",
                           gorev_listesi[i].ID, gorev_listesi[i].GELIS_ZAMANI, gorev_listesi[i].SURE,
                           gorev_listesi[i].ONCELIK, durum_metinleri[gorev_listesi[i].durum],
                           teknisyen_gosterim, gorev_listesi[i].kalan_sure);
                }
                printf("----------------------------------------------------------------------------------------------\n");
                break;

            case 3: {
                printf("\n--- GOREV DURUM OZETI ---\n");
                int durum_sayilari[4] = {0, 0, 0, 0};
                for (int i = 0; i < gorev_sayisi; i++) {
                    durum_sayilari[gorev_listesi[i].durum]++;
                }

                // Görev sayılarını yazdır
                printf("Toplam Gorev Sayisi : %d\n", gorev_sayisi);
                printf("Henuz Gelmedi       : %d\n", durum_sayilari[0]);
                printf("Bekliyor            : %d\n", durum_sayilari[1]);
                printf("Devam Ediyor        : %d\n", durum_sayilari[2]);
                printf("Tamamlandi          : %d\n", durum_sayilari[3]);
                printf("-------------------------------------------------------\n");

                int istenen_durum;
                printf("Detayli listesini gormek istediginiz durumu seciniz:\n");
                printf("0- Henuz Gelmedi\n1- Bekliyor\n2- Devam Ediyor\n3- Tamamlandi\n4- Menuye Don\nSeciminiz: ");
                scanf("%d", &istenen_durum);

                if(istenen_durum == 4) break;
                if(istenen_durum < 0 || istenen_durum > 3) {
                    printf("Gecersiz durum secimi!\n");
                    break;
                }

                printf("\n--- %s DURUMUNDAKI GOREVLER ---\n", durum_metinleri[istenen_durum]);
                printf("%-6s %-14s %-12s %-9s %-15s %-18s %-12s\n",
                       "ID", "Gelis Zamani", "Toplam Sure", "Oncelik", "Durum", "Guncel Teknisyen", "Kalan Sure");
                printf("----------------------------------------------------------------------------------------------\n");

                for (int i = 0; i < gorev_sayisi; i++) {
                    if(gorev_listesi[i].durum == istenen_durum) {
                        char teknisyen_gosterim[5];
                        if (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0) strcpy(teknisyen_gosterim, "-");
                        else strcpy(teknisyen_gosterim, gorev_listesi[i].guncel_teknisyen);

                        printf("%-6d %-14d %-12d %-9d %-15s %-18s %-12d\n",
                               gorev_listesi[i].ID, gorev_listesi[i].GELIS_ZAMANI, gorev_listesi[i].SURE,
                               gorev_listesi[i].ONCELIK, durum_metinleri[gorev_listesi[i].durum],
                               teknisyen_gosterim, gorev_listesi[i].kalan_sure);
                    }
                }
                printf("----------------------------------------------------------------------------------------------\n");
                break;
            }

            case 4: {
                int aranan_id;
                printf("\n--- GOREV ID SORGULAMA ---\n");
                printf("Sorgulamak istediginiz gorevin ID'sini giriniz: ");
                scanf("%d", &aranan_id);

                int id_bulundu = 0;
                for(int i = 0; i < gorev_sayisi; i++) {
                    if(gorev_listesi[i].ID == aranan_id) {
                        id_bulundu = 1;
                        printf("\n--- GOREV BILGILERI ---\n");
                        printf("ID                  : %d\n", gorev_listesi[i].ID);
                        printf("Gorev Turu          : %s\n", gorev_listesi[i].GOREV_TURU);
                        printf("Konum               : %s\n", gorev_listesi[i].KONUM);
                        printf("Gelis Zamani        : %d\n", gorev_listesi[i].GELIS_ZAMANI);
                        printf("Toplam Sure         : %d\n", gorev_listesi[i].SURE);
                        printf("Oncelik             : %d\n", gorev_listesi[i].ONCELIK);
                        printf("Durum               : %s\n", durum_metinleri[gorev_listesi[i].durum]);

                        if (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0) printf("Guncel Teknisyen    : -\n");
                        else printf("Guncel Teknisyen    : %s\n", gorev_listesi[i].guncel_teknisyen);

                        printf("Kalan Sure          : %d\n", gorev_listesi[i].kalan_sure);

                        // Başlama ve bitiş zamanı kontrolleri
                        if(gorev_listesi[i].ilk_baslama_zamani == -1) printf("Ilk Baslama Zamani  : -\n");
                        else printf("Ilk Baslama Zamani  : %d\n", gorev_listesi[i].ilk_baslama_zamani);

                        if(gorev_listesi[i].bitis_zamani == -1) printf("Tamamlanma Zamani   : -\n");
                        else printf("Tamamlanma Zamani   : %d\n", gorev_listesi[i].bitis_zamani);

                        break;
                    }
                }
                if(id_bulundu == 0) printf("Girdiginiz %d ID'li gorev sistemde bulunamadi!\n", aranan_id);
                break;
            }

            case 5: {
                printf("\n--- TEKNISYENLERIN DURUMU ---\n");

                // Algoritmaya göre tablo başlıklarını ayarla
                if (secili_algoritma == 3) {
                    printf("%-10s %-12s %-10s %-18s %-18s %-20s\n",
                           "Teknisyen", "Durum", "Gorev ID", "Gorevin Kalan Sure", "Toplam Calisma", "Dilimde Kalan Sure");
                    printf("--------------------------------------------------------------------------------------------\n");
                } else {
                    printf("%-10s %-12s %-10s %-18s %-18s\n",
                           "Teknisyen", "Durum", "Gorev ID", "Gorevin Kalan Sure", "Toplam Calisma");
                    printf("----------------------------------------------------------------------\n");
                }

                for (int i = 0; i < 4; i++) {
                    char durum_str[15];
                    char id_str[10];
                    char kalan_sure_str[10];
                    char dilim_str[15];

                    if (teknikerler[i].Durum == 0) {
                        strcpy(durum_str, "Bos");
                        strcpy(id_str, "-");
                        strcpy(kalan_sure_str, "-");
                        strcpy(dilim_str, "-");
                    } else {
                        // Geçiş süresi kontrolü
                        if (teknikerler[i].Gecis_Suresi_Kaldi > 0) strcpy(durum_str, "Geciste");
                        else strcpy(durum_str, "Mesgul");

                        sprintf(id_str, "%d", teknikerler[i].Gorev_ID);

                        int g_kalan = 0;
                        for (int j = 0; j < gorev_sayisi; j++) {
                            if (gorev_listesi[j].ID == teknikerler[i].Gorev_ID) {
                                g_kalan = gorev_listesi[j].kalan_sure;
                                break;
                            }
                        }
                        sprintf(kalan_sure_str, "%d", g_kalan);

                        if (secili_algoritma == 3) {
                            if (teknikerler[i].Gecis_Suresi_Kaldi > 0) strcpy(dilim_str, "Beklemede");
                            else sprintf(dilim_str, "%d", quantum_degeri - teknikerler[i].Dilimde_Kalan_Sure);
                        }
                    }

                    if (secili_algoritma == 3) {
                        printf("%-10s %-12s %-10s %-18s %-18d %-20s\n",
                               teknikerler[i].isim, durum_str, id_str, kalan_sure_str, teknikerler[i].Calisma_Suresi, dilim_str);
                    } else {
                        printf("%-10s %-12s %-10s %-18s %-18d\n",
                               teknikerler[i].isim, durum_str, id_str, kalan_sure_str, teknikerler[i].Calisma_Suresi);
                    }
                }

                if (secili_algoritma == 3) printf("--------------------------------------------------------------------------------------------\n");
                else printf("----------------------------------------------------------------------\n");

                break;
            }

            case 6:
                // Algoritma seçimi kontrolü
                if (secili_algoritma == 0) {
                    printf("\nHata: Lutfen once 1. secenekten bir algoritma seciniz!\n");
                    break;
                }
                zaman_ilerletme_ve_atama(gorev_listesi, gorev_sayisi, teknikerler, secili_algoritma, quantum_degeri, &sanal_zaman, &siramatik);
                printf("Zaman 1 birim ilerletildi. Guncel zaman: %d\n", sanal_zaman);
                break;

            case 7: {
                if (secili_algoritma == 0) {
                    printf("\nHata: Lutfen once 1. secenekten bir algoritma seciniz!\n");
                    break;
                }
                int n;
                printf("Kac birim ilerletmek istiyorsunuz: ");
                scanf("%d", &n);
                // Belirtilen birim kadar zamanı ilerlet
                for(int i = 0; i < n; i++) {
                    zaman_ilerletme_ve_atama(gorev_listesi, gorev_sayisi, teknikerler, secili_algoritma, quantum_degeri, &sanal_zaman, &siramatik);
                }
                printf("Zaman %d birim ilerletildi. Guncel zaman: %d\n", n, sanal_zaman);
                break;
            }

            case 8: {
                if (secili_algoritma == 0) {
                    printf("\nHata: Lutfen once 1. secenekten bir algoritma seciniz!\n");
                    break;
                }
                int hedef;
                printf("Gitmek istediginiz zamani giriniz: ");
                scanf("%d", &hedef);

                // Hedef zaman geçmişteyse sistemi sıfırla
                if (hedef < sanal_zaman) {
                    simulasyonu_sifirla(gorev_listesi, gorev_sayisi, teknikerler, &sanal_zaman, &siramatik);
                }

                while (sanal_zaman < hedef) {
                    zaman_ilerletme_ve_atama(gorev_listesi, gorev_sayisi, teknikerler, secili_algoritma, quantum_degeri, &sanal_zaman, &siramatik);
                }
                printf("Hedef zamana ulasildi. Guncel zaman: %d\n", sanal_zaman);
                break;
            }

            case 9: {
                printf("\n--- ISTATISTIKLER ---\n");

                int tamamlanan_gorev_sayisi = 0;
                int makespan = 0;
                float toplam_bekleme = 0.0;
                float toplam_sistem_sure = 0.0;

                for (int i = 0; i < gorev_sayisi; i++) {
                    if (gorev_listesi[i].durum == TAMAMLANDI) {
                        tamamlanan_gorev_sayisi++;

                        // Bekleme süresi hesabı
                        int bekleme = gorev_listesi[i].bitis_zamani - gorev_listesi[i].GELIS_ZAMANI - gorev_listesi[i].SURE;
                        toplam_bekleme += bekleme;

                        // Sistemde kalma süresi hesabı
                        int sistem_sure = gorev_listesi[i].bitis_zamani - gorev_listesi[i].GELIS_ZAMANI;
                        toplam_sistem_sure += sistem_sure;

                        // Makespan hesabı
                        if (gorev_listesi[i].bitis_zamani > makespan) {
                            makespan = gorev_listesi[i].bitis_zamani;
                        }
                    }
                }

                printf("Tamamlanan Gorev Sayisi : %d / %d\n", tamamlanan_gorev_sayisi, gorev_sayisi);

                if (tamamlanan_gorev_sayisi > 0) {
                    if (tamamlanan_gorev_sayisi == gorev_sayisi) printf("Makespan                : %d\n", makespan);
                    else printf("Makespan                : Henuz tum isler bitmedi (Su ana kadar en gec bitis: %d)\n", makespan);

                    printf("Ortalama Bekleme Suresi : %.3f\n", toplam_bekleme / tamamlanan_gorev_sayisi);
                    printf("Ortalama Sistemde Kalma : %.3f\n", toplam_sistem_sure / tamamlanan_gorev_sayisi);
                } else {
                    printf("Makespan                : Hesaplanamaz\n");
                    printf("Ortalama Bekleme Suresi : Hesaplanamaz\n");
                    printf("Ortalama Sistemde Kalma : Hesaplanamaz\n");
                }

                printf("\n- Kullanim Oranlari -\n");
                if (sanal_zaman > 0) {
                    int toplam_calisma_genel = 0;
                    for (int i = 0; i < 4; i++) {
                        // Teknisyen kullanım oranı hesabı
                        float kullanim = ((float)teknikerler[i].Calisma_Suresi / sanal_zaman) * 100.0;
                        printf("%s Kullanim Orani      : %%%.3f\n", teknikerler[i].isim, kullanim);
                        toplam_calisma_genel += teknikerler[i].Calisma_Suresi;
                    }
                    // Genel kullanım oranı hesabı
                    float genel_kullanim = ((float)toplam_calisma_genel / (4 * sanal_zaman)) * 100.0;
                    printf("Genel Kullanim Orani    : %%%.3f\n", genel_kullanim);
                } else {
                    printf("Teknisyen Kullanim Orani: Hesaplanamaz (Zaman 0)\n");
                    printf("Genel Kullanim Orani    : Hesaplanamaz (Zaman 0)\n");
                }

                printf("-------------------------------------------------------\n");
                break;
            }

            case 10:
                printf("\nSimulasyon sifirlaniyor...\n");
                simulasyonu_sifirla(gorev_listesi, gorev_sayisi, teknikerler, &sanal_zaman, &siramatik);
                printf("Simulasyon basariyla zaman 0'a sifirlandi ve tum veriler temizlendi.\n");
                break;

            case 11:
                printf("\nProgramdan cikiliyor...\n");
                return 0;

            default:
                printf("Gecersiz secim! Lutfen 1-11 arasi bir deger giriniz.\n");
                break;
        }
    }
    return 0;
}
