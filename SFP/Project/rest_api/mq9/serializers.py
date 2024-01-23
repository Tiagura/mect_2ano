from rest_framework import serializers
from .models import MQ9

class MQ9Serializer(serializers.ModelSerializer):
    class Meta:
        model = MQ9
        fields = '__all__'

    def create(self, validated_data):
        return MQ9.objects.create(**validated_data)
        
    def save(self):
        return super().save()
    
    def to_representation(self, instance):
        return {
            'id': instance.id,
            'co': instance.co,
            'time': instance.time
        }
    
    def serialize(self):
        return super().serialize()