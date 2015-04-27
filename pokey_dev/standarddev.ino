//thank you john d cook



void clear_std(Stddev* s) {
    s->m_n = 0;
}

void push(Stddev* s, float x) {
    s->m_n++;

    // See Knuth TAOCP vol 2, 3rd edition, page 232
    if (s->m_n == 1) {
        s->m_oldM = s->m_newM = x;
        s->m_oldS = 0.0;
    } else {
        s->m_newM = s->m_oldM + (x - s->m_oldM)/s->m_n;
        s->m_newS = s->m_oldS + (x - s->m_oldM)*(x - s->m_newM);

        // set up for next iteration
        s->m_oldM = s->m_newM; 
        s->m_oldS = s->m_newS;
    }
}


float mean(Stddev* s) {
    return (s->m_n > 0) ? s->m_newM : 0.0;
}

float var(Stddev* s) {
    return ( (s->m_n > 1) ? s->m_newS/(s->m_n - 1) : 0.0 );
}

float std_dev(Stddev* s) {
    return sqrt( var(s) );
}
