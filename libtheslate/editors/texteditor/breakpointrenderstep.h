#ifndef BREAKPOINTRENDERSTEP_H
#define BREAKPOINTRENDERSTEP_H

#include <rendering/texteditorperlinerenderstep.h>

class BreakpointRenderStep : public TextEditorPerLineRenderStep {
	Q_OBJECT
public:
	explicit BreakpointRenderStep(TextEditor* parent = nullptr);

	// Inherited via TextEditorPerLineRenderStep
	virtual RenderSide renderSide() const override;
	virtual int renderWidth() const override;
	virtual QString stepName() const override;
	virtual uint priority() const override;
	virtual QString renderStack() const override;
	virtual void paintLine(int line, QPainter* painter, QRect outputBounds, QRect redrawBounds) override;
};

#endif // BREAKPOINTRENDERSTEP_H