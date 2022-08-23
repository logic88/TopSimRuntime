#include "stdafx.h"

#include "HRCheckableGroupBox.h"
#include <QVBoxLayout>


HRCheckableGroupBox::HRCheckableGroupBox(QWidget * parent)
	:QGroupBox(parent)
{
	setObjectName("HRCheckableGroupBox");

	setCheckable(true);
	connect(this, SIGNAL(clicked(bool)), this, SLOT(OnCheckedSlot(bool)));
}

void HRCheckableGroupBox::OnCheckedSlot(bool checked)
{
	QLayout * lyt = layout();

	if (lyt)
	{
		int count = lyt->count();

		for (size_t i = 0; i < count; ++i)
		{
			QLayoutItem * item = lyt->itemAt(i);
			if (item && item->widget())
			{
				item->widget()->setVisible(checked);
			}
		}
	}

	setFlat(!checked);
}

void HRCheckableGroupBox::SetWidgetsEnabled(bool Enabled)
{
	QLayout * lyt = layout();

	if (lyt)
	{
		int count = lyt->count();

		for (size_t i = 0; i < count; ++i)
		{
			QLayoutItem * item = lyt->itemAt(i);
			if (item && item->widget())
			{
				item->widget()->setEnabled(Enabled);
			}
		}
	}
}