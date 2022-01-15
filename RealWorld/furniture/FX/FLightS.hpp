#pragma once
#include "FStatic.hpp"
#include "../../world/DestroyTicket.hpp"

class FLightS: public FStatic{
public:
	FLightS(const FStatic& f) : FStatic(f) {

	};

	void build() override;

	void destroy() override;
protected:
	DestroyTicket<StaticLight> p_sl;
};