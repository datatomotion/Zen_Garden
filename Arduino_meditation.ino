/*
  ──────────────────────────────────────────────────────────────────────────────
   Data To Motion – Jardin Zen motorisé (28BYJ-48 + ULN2003 + Arduino UNO)
  ──────────────────────────────────────────────────────────────────────────────
   OBJECTIF
   - Faire tourner un moteur pas à pas 28BYJ-48 de manière fluide (demi-pas).
   - Régler la VITESSE avec un potentiomètre branché sur A0.

   MATERIEL
   - Arduino UNO
   - Moteur pas à pas 28BYJ-48 + driver ULN2003
   - Potentiomètre 10 kΩ (recommandé)
   - Alimentation 5 V (moteur) + GND commun avec l’Arduino

   CABLAGE (exemple courant – peut varier selon vos cartes)
   - ULN2003 IN1 → D8
   - ULN2003 IN2 → D9
   - ULN2003 IN3 → D10
   - ULN2003 IN4 → D11
   - ULN2003 VCC → +5 V (de préférence alimentation externe)
   - ULN2003 GND → GND Arduino
   - Potentiomètre : broche centrale → A0, les deux autres → +5V et GND

   REMARQUES
   - Si le moteur "tremble" sans tourner : l’ordre des IN1..IN4 ne correspond
     pas à la séquence. Inversez l’ordre des fils (par ex. 1-3-2-4) jusqu’à
     obtenir une rotation régulière.
   - Pour inverser le SENS : changez la variable 'dir' de +1 à -1.
   - La demi-pas active parfois 2 bobines → plus de couple que "wave drive".
   - La lecture du potentiomètre est lissée pour éviter les à-coups.

   Auteur : Data To Motion
   Licence : usage éducatif
  ──────────────────────────────────────────────────────────────────────────────
*/

/// ----------------------------- Réglages pins --------------------------------
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define POT_PIN A0

/// --------------------- Séquence DEMI-PAS (8 états) --------------------------
/// Chaque ligne = état des 4 bobines (IN1..IN4)
const uint8_t HALFSEQ[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};

/// Index de pas (0..7) et direction (+1 = horaire, -1 = antihoraire)
int stepIndex = 0;
int dir = +1;   // mettez -1 pour inverser le sens

/// Lissage de la vitesse (anti-jitter du potentiomètre)
int smoothDelayMs = 20;     // valeur de départ (ms)
const float SMOOTH_ALPHA = 0.2f; // 0..1 | plus grand = réagit plus vite

/// ------------------------------- Setup --------------------------------------
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  // pinMode(POT_PIN, INPUT); // inutile : analogRead le fait par défaut
  setCoils(HALFSEQ[stepIndex]); // état initial propre
}

/// ------------------------------- Loop ---------------------------------------
void loop() {
  // 1) Appliquer l’état de bobines correspondant à stepIndex
  setCoils(HALFSEQ[stepIndex]);

  // 2) Avancer d’un demi-pas dans la direction choisie
  stepIndex += dir;
  if (stepIndex > 7) stepIndex = 0;
  if (stepIndex < 0) stepIndex = 7;

  // 3) Lire le potentiomètre et convertir en délai (ms)
  //    On mappe 0..1023 vers une plage lente→rapide (120..3 ms)
  int targetDelayMs = map(analogRead(POT_PIN), 0, 1023, 120, 3);
  targetDelayMs = constrain(targetDelayMs, 3, 200); // bornes de sécurité

  // 4) Lisser pour éviter les variations brusques (filtre passe-bas)
  smoothDelayMs = (int)( (1.0f - SMOOTH_ALPHA) * smoothDelayMs
                       + SMOOTH_ALPHA * targetDelayMs );

  // 5) Attendre avant le prochain demi-pas
  delay(smoothDelayMs);
}

/// --------------------------- Fonctions utiles -------------------------------

/// Applique l’état des 4 bobines d’après une ligne de HALFSEQ
void setCoils(const uint8_t s[4]) {
  digitalWrite(IN1, s[0]);
  digitalWrite(IN2, s[1]);
  digitalWrite(IN3, s[2]);
  digitalWrite(IN4, s[3]);
}
