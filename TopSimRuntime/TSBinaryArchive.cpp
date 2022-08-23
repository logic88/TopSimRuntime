#include "stdafx.h"

#include "HRLineEditWithCancleBtn.h"

HRLabel::HRLabel( QWidget* parent /*= 0*/ )
	:QLabel(parent)
	,_LabelUrl(TSString2QString(":/Resources/images/cleartext.png"))
{
	QSize size = QSize(16,20);
	setFixedSize(size);
	setFocusPolicy(Qt::StrongFocus);
	setToolTip(FromAscii("Çå¿ÕÄÚÈÝ"));
	QPixmap oriPixmap = QPixmap(_LabelUrl);
	QPixmap newPixmap = oriPixmap.scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
	setPixmap(newPixmap);
}

void HRLabel::focusInEvent( QFocusEvent* event )
{
	this->setCursor(Qt::ArrowCursor);
	QLabel::focusInEvent(event);
}

void HRLabel::mousePressEvent( QMouseEvent* event )
{
	if (event->buttons() & Qt::LeftButton)
	{
		emit(clicked());
	}
	QLabel::mousePressEvent(event);
}

HRLabel::~HRLabel()
{
}

HRLineEditWithCancleBtn::HRLineEditWithCancleBtn( QWidget* parent /*= 0*/ )
	:QLineEdit(parent)
{
	_Label = new HRLabel(this);
	_Label->setHidden(true);

	this->setTextMargins(0,1,_Label->sizeHint().width(),1);
	connect(_Label,SIGNAL(clicked()),this,SIGNAL(clearContents()));
	connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(OnTextChanged(const QString&)));
}

HRLineEditWithCancleBtn::~HRLineEditWithCancleBtn()
{
	if (_Label)
	{
		delete _Label;
		_Label = NULL;
	}
}

void HRLineEditWithCancleBtn::resizeEvent( QResizeEvent* event )
{
	_Label->move(this->width()-_Label->width()-2,1);
	QLineEdit::resizeEvent(event);
}

void HRLineEditWithCancleBtn::OnTextChanged( const QString& text )
{
	if (text.isEmpty())
	{
		_Label->setHidden(true);
	}
	else
	{
		_Label->setHidden(false);	

		this->setFocus();
	}
}

