#ifndef NULLMODEL_H
#define NULLMODEL_H

#include "model/model.h"

class NullModel : public Model
{
public:
    NullModel();
    virtual ~NullModel();
    virtual void initialize(const QStringList& options,PositionQuery* position, QRectF bounds);
    virtual void apply(const TactileDisplay* display, QVector<FrequencyTable>& tables);

private:
    PositionQuery* position = nullptr;    
};

#endif // NULLMODEL_H
