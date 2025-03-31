from django.db import models

# Create your models here.


class Dummy(models.Model):

    class Meta:
        app_label = 'dummyapp'
        pass

    name = models.CharField(max_length=64)
    value = models.IntegerField(default=0)

    pass

