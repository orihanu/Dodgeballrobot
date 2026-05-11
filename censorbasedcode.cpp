// --- HC-SR04 Sensör Pinleri ---
const int trigOrta = 12;
const int echoOrta = 11;

const int trigSag = A0;
const int echoSag = A1;

const int trigSol = A2;
const int echoSol = A3;

const int mesafeSiniri = 20; // 20 cm'den yaklaşan bir şeyden kaçacak

// --- L298N Motor Sürücü Pinleri ---
const int enA = 10; 
const int in1 = 9;  
const int in2 = 8;  

const int in3 = 7;  
const int in4 = 6;  
const int enB = 5;  

// Robotun kararını aklında tutması için hafıza değişkeni (0: Kararsız, 1: Sağa, 2: Sola)
int kacisYonu = 0; 

void setup() {
  randomSeed(analogRead(A4)); // Rastgelelik için

  // Sensör pinleri
  pinMode(trigOrta, OUTPUT); pinMode(echoOrta, INPUT);
  pinMode(trigSag, OUTPUT); pinMode(echoSag, INPUT);
  pinMode(trigSol, OUTPUT); pinMode(echoSol, INPUT);

  // Motor pinleri
  pinMode(enA, OUTPUT); pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT); pinMode(enB, OUTPUT);

  motorDur();
  Serial.begin(9600);
}

void loop() {
  // Sensörlerin mesafelerini ölçüyoruz
  long mesafeOrta = mesafeOlc(trigOrta, echoOrta);
  long mesafeSag  = mesafeOlc(trigSag, echoSag);
  long mesafeSol  = mesafeOlc(trigSol, echoSol);

  // Sensörlerin durumlarını belirliyoruz
  bool ortaAlgiladi = (mesafeOrta > 0 && mesafeOrta < mesafeSiniri);
  bool sagAlgiladi  = (mesafeSag > 0 && mesafeSag < mesafeSiniri);
  bool solAlgiladi  = (mesafeSol > 0 && mesafeSol < mesafeSiniri);

  // --- YAN YAN KAÇIŞ MANTIĞI ---

  // 1. Durum: Hiçbir sensör algılamıyorsa robot dursun ve hafızayı sıfırlasın
  if (!ortaAlgiladi && !sagAlgiladi && !solAlgiladi) {
    kacisYonu = 0; // Tehlike geçti, kararı unut
    motorDur();
  }
  
  // 2. Durum: SADECE Orta sensör algılıyorsa (Tam karşıdan tehdit)
  else if (ortaAlgiladi && !sagAlgiladi && !solAlgiladi) {
    
    // Eğer daha önce bir yön seçilmemişse (kacisYonu 0 ise) rastgele bir yön seç
    if (kacisYonu == 0) {
      kacisYonu = random(1, 3); // 1 (Sağ) veya 2 (Sol) değerini seçer
    }
    
    // Seçilen yöne göre kaçmaya devam et
    if (kacisYonu == 1) {
      Serial.println("Karsi Tehdit: Karar Verildi -> SAGA KAY!");
      sagaKay();
    } else {
      Serial.println("Karsi Tehdit: Karar Verildi -> SOLA KAY!");
      solaKay();
    }
  }
  
  // 3. Durum: Sağdan veya Sağ+Ortadan hareket varsa -> SOLA KAYARAK KAÇ
  else if (sagAlgiladi && !solAlgiladi) {
    kacisYonu = 0; // Farklı bir senaryoya geçildi, ortadan kaçış hafızasını sıfırla
    Serial.println("Sag Tehdit: SOLA KAY!");
    solaKay();
  }
  
  // 4. Durum: Soldan veya Sol+Ortadan hareket varsa -> SAĞA KAYARAK KAÇ
  else if (solAlgiladi && !sagAlgiladi) {
    kacisYonu = 0; // Farklı bir senaryoya geçildi, ortadan kaçış hafızasını sıfırla
    Serial.println("Sol Tehdit: SAGA KAY!");
    sagaKay();
  }
  
  // 5. Durum: Hem Sağ Hem Sol algılıyorsa
  else if (solAlgiladi && sagAlgiladi) {
    kacisYonu = 0; 
    Serial.println("Kusatildi: KACACAK YER YOK, DUR!");
    motorDur();
  }

  delay(50); 
}

// ==========================================
// ALT FONKSİYONLAR 
// ==========================================

long mesafeOlc(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long sure = pulseIn(echo, HIGH, 30000); 
  
  if (sure == 0) return 999; 
  return (sure / 2) / 29.1;
}

// Yengeç Yürüyüşü Motor Fonksiyonları

void solaKay() {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); 
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW); 
  analogWrite(enA, 255); analogWrite(enB, 255);
}

void sagaKay() {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); 
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH); 
  analogWrite(enA, 255); analogWrite(enB, 255);
}

void motorDur() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0); analogWrite(enB, 0);
}