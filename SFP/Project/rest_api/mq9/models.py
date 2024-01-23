from django.db import models

class MQ9(models.Model):
    id = models.AutoField(primary_key=True)
    co = models.FloatField()
    time = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f'{self.id} {self.mq9} {self.time}'

    class Meta:
        verbose_name = 'MQ9'
        verbose_name_plural = 'MQ9'
        ordering = ['id']