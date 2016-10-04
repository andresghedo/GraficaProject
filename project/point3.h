/*
 *  CLASSE Point3
 * 
 *  Punto o vettore in 3D, implementa le operazione fra punti 
 * 
 */
class Point3 {
public:

    /* coordinate 3D */
    float coord[3];

    float X() const {
        return coord[0];
    }

    float Y() const {
        return coord[1];
    }

    float Z() const {
        return coord[2];
    }
    
    /* costruttore con xyz */
    Point3(float x, float y, float z) {
        coord[0] = x;
        coord[1] = y;
        coord[2] = z;
    }

    /* costruttore vuoto */
    Point3() {
        coord[0] = coord[1] = coord[2] = 0;
    }


    /* normalizzazione */
    Point3 Normalize() const {
        return (*this) / modulo();
    }

    /* modulo */
    float modulo() const {
        return
        sqrt(coord[0] * coord[0] + coord[1] * coord[1] + coord[2] * coord[2]);
    }

    /* operatore binario - divisione per uno scalare f */
    Point3 operator/(float f) const {
        return Point3(
                coord[0] / f,
                coord[1] / f,
                coord[2] / f
                );
    }

    /* inversione verso del vettore */
    Point3 operator-()const {
        return Point3(
                -coord[0],
                -coord[1],
                -coord[2]
                );
    }

    /* differenza tra punti */
    Point3 operator-(const Point3 &a)const {
        return Point3(
                coord[0] - a.coord[0],
                coord[1] - a.coord[1],
                coord[2] - a.coord[2]
                );
    }

    /* somma tra vettori */
    Point3 operator+(const Point3 &a)const {
        return Point3(
                coord[0] + a.coord[0],
                coord[1] + a.coord[1],
                coord[2] + a.coord[2]
                );
    }


    /* operatore % */
    Point3 operator%(const Point3 &a)const {
        return Point3(
                coord[1] * a.coord[2] - coord[2] * a.coord[1],
                -(coord[0] * a.coord[2] - coord[2] * a.coord[0]),
                coord[0] * a.coord[1] - coord[1] * a.coord[0]
                );
    }

    /* mandare il punto come vertice di OpenGl */
    void SendAsVertex() const {
        glVertex3fv(coord);
    }

    /* mandare il punto come normale di OpenGl */
    void SendAsNormal() const {
        glNormal3fv(coord);
    }

    /* setta X del punto/vettore */
    void setX(float x) {
        coord[0] = x;
    }

    /* setta Y del punto/vettore */
    void setY(float y) {
        coord[1] = y;
    }

    /* setta Z del punto/vettore */
    void setZ(float z) {
        coord[2] = z;
    }
};

/* definizione di sinonimo */
typedef Point3 Vector3;

inline void glTranslate(Point3 v) {
    glTranslatef(v.X(), v.Y(), v.Z());
}
