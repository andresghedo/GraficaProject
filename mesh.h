/*
 *  CLASSE Vertex
 */
class Vertex {
public:
    Point3 p;

    Vector3 n;
};

/*
 *  CLASSE Edge
 */
class Edge {
public:
    Vertex* v[2];
};

/*
 *  CLASSE Face
 */
class Face {
public:
    Vertex* v[3];

    Face(Vertex* a, Vertex* b, Vertex* c) {
        v[0] = a;
        v[1] = b;
        v[2] = c;
    }

    Vector3 n;

    void ComputeNormal() {
        n = -((v[1]->p - v[0]->p) % (v[2]->p - v[0]->p)).Normalize();
    }

};

/*
 *  CLASSE Mesh
 */
class Mesh {
    /* vettore di vertici */
    std::vector<Vertex> v;
    /* vettore di facce */
    std::vector<Face> f;
    /* vettore di edge */
    std::vector<Edge> e;

public:

    /* costruttore che carica già la mesh dal file filename */
    Mesh(char *filename) {
        LoadFromObj(filename);
        ComputeNormalsPerFace();
        ComputeNormalsPerVertex();
        ComputeBoundingBox();
    }

    /* manda a schermo la mesh Normali x Faccia */
    void RenderNxF();
    /* manda a schermo la mesh Normali x Vertice */
    void RenderNxV();
    /* manda a schermo la mesh in wireframe */
    void RenderWire();
    /* carica la mesh da un file OFF */
    bool LoadFromObj(char* filename);

    void ComputeNormalsPerFace();

    void ComputeNormalsPerVertex();

    void ComputeBoundingBox();

    /* centro del axis aligned bounding box */
    Point3 Center() {
        return (bbmin + bbmax) / 2.0;
    };

    Point3 bbmin, bbmax; 
};
