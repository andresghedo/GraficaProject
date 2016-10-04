class Car {
    /* disegna tutte le parti della macchina */
    void RenderAllParts(bool usecolor) const;

public:
    /* inizializzazione */
    void Init();
    /* disegno a schermo */
    void Render() const;
    /* passo di fisica */
    void DoStep(bool LeftKey, bool RightKey, bool AccKey, bool DecKey);

    /* costruttore */
    Car() {
        Init();
    }

    /* STATO DELLA MACCHINA VARIABILE */
    /* (DoStep fa evolvere queste variabili nel tempo) */
    /* XYZ Car e angolo */
    float px, py, pz, facing;
    /* stato interno */
    float mozzoA, mozzoP, sterzo;
    /* velocità attuale */
    float vx, vy, vz;

    /* STATS DELLA MACCHINA COSTANTI */
    float velSterzo, velRitornoSterzo, accMax, attrito,
    raggioRuotaA, raggioRuotaP, grip,
    attritoX, attritoY, attritoZ;

private:
    /* accensione fari */
    void DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const;
};
